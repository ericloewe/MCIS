/* 
Copyright (c) 2018, Eric Loewenthal
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the organization nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <libconfig.h++>
#include <arpa/inet.h>
#include "include/MCIS_MB_interface.h"
#include "include/MCIS_config.h"

// The MCIS parameters config file will be loaded from here:
#define appConfigFilename "MCISinit.cfg"
/* Defaults */
//#define MB_IP 0xC0A81405  //192.168.20.5 in hex
//#define MB_PORT 991    //CHANGE ME!
//#define XPLANE_RECV_PORT 49000 //CHANGE ME!

#define LOCAL_PORT 10500
#define MDA_CONF_FILENAME "MDAconfig.bin"
#define MDA_LOGNAME "mdalog"
#define MDA_LOGEXT  ".csv"

void sig_handler(int signo);

bool cont = true;

int main(int argc, char *argv[])
{
    /* Configuration parameter variables */
    std::string mdaConfigFilename = MDA_CONF_FILENAME,
                MBaddrStr;
    uint32_t MBaddr, MBport, localPort = LOCAL_PORT, XPport;
    std::string MDAlogFilepath, 
                MDAlogFilename = MDA_LOGNAME,
                MDAlogFileext  = MDA_LOGEXT;
    bool subgrav = true;


     /*
     *  Load application configuration parameters from MCISinit.cfg
     */
    libconfig::Config appConf;
    std::cout << "Loading application configuration...   " << std::endl;
    try // Load the config so we can lookup settings
    {
        appConf.readFile(appConfigFilename);
    }
    catch(const libconfig::FileIOException& e)
    {
        std::cout << "Failed to open application configuration file ";
        std::cout << appConfigFilename << std::endl;
        std::cout << e.what() << std::endl;
        return 0;
    }
    catch(const libconfig::ParseException& e)
    {
        std::cout << "Failed to parse application configuration file ";
        std::cout << appConfigFilename << std::endl;
        std::cout << e.what() << std::endl;
        return 0;
    }

    try // Lookup essential configurations for which there are no defaults
    {
        MBaddrStr = (const char *)appConf.lookup("MB.IP_addr");
        MBport = appConf.lookup("MB.port");
        XPport = appConf.lookup("XP.port");
    }
    catch(const libconfig::SettingTypeException& e)
    {
        std::cout << "Error: Setting from file has wrong type." << std::endl;;
        std::cout << "Setting: "<< e.getPath() << std::endl;
        return 0;
    }
    catch(const libconfig::SettingNotFoundException& e)
    {
        std::cout << "Error: Setting is missing from file." << std::endl;;
        std::cout << "Setting: "<< e.getPath() << std::endl;
        return 0;
    }
    //Transform the MB IP address string into something machine-readable
    struct in_addr addrStruct;
    if (inet_pton(AF_INET, MBaddrStr.c_str(), (void *)&addrStruct) != 1)
    {
        // String doesn't contain a valid IP address, we need to bail out
        std::cout << "Error: Invalid MB IP address: ";
        std::cout << MBaddrStr << std::endl;
        return 0;
    }
    //It's in Network Byte Order, we need to pass it to the MBinterface in
    //Host Byte Order, so we need to ntohl it.
    MBaddr = ntohl(addrStruct.s_addr);

    // Lookup optional configurations for which we have defaults
    if (!appConf.lookupValue("MB.local_port", localPort))
    {
        std::cout << "Using default local port: " << localPort << std::endl;
    }
    if (!appConf.lookupValue("MCIS.MDA_config", mdaConfigFilename))
    {
        std::cout << "Using default MDA config file: " << mdaConfigFilename << std::endl;
    }
    if (!appConf.lookupValue("MCIS.MDA_logfilename", MDAlogFilename))
    {
        std::cout << "Using default MDA log filename: " << MDAlogFilename << std::endl;
    }
    appConf.lookupValue("MCIS.subtract_gravity", subgrav);
    if (!subgrav)
    {
        std::cout << "WARNING:  GRAVITY SUBTRACTION IS DISABLED IN THE CONFIG FILE." << std::endl;
        std::cout << "          IT IS HIGHLY RECOMMENDED THAT GRAVITY SUBTRACTION" << std::endl;
        std::cout << "          BE ENABLED. DISABLING IT SHOULD ONLY BE DONE FOR" << std::endl;
        std::cout << "          RESEARCH PURPOSES. To continue, press return." << std::endl;
        getchar();
    }


    MCISconfig config;
    std::fstream MDA_log;


    /* 
     * Load the MDA config and handle errors
     */
    try
    {
        config.load(mdaConfigFilename);
    }
    catch (const littleEndianConfigException&)
    {
        std::cout << "Error: Cannot load MDA config file ";
        std::cout << mdaConfigFilename << std::endl;
        std::cout << "The MDA config file uses an old, little-endian format." << std::endl;
        std::cout << "Only v05 big-endian config files are supported." << std::endl;
        return 0;
    }
    catch (const oldConfigTypeException&)
    {
        std::cout << "Error: Cannot load MDA config file ";
        std::cout << mdaConfigFilename << std::endl;
        std::cout << "The MDA config file uses an old, unsupported format." << std::endl;
        return 0;
    }
    catch (const unsupportedConfigTypeException&)
    {
        std::cout << "Error: Cannot load MDA config file ";
        std::cout << mdaConfigFilename << std::endl;
        std::cout << "The MDA config file uses an unknown format." << std::endl;
        std::cout << "The header may be corrupt." << std::endl;
        std::cout << "This would be a problem at time of file generation," << std::endl;
        std::cout << "as the file's CRC is valid." << std::endl;
        return 0;
    }
    catch (const badLengthException& except)
    {
        std::cout << "Error: Cannot load MDA config file ";
        std::cout << mdaConfigFilename << std::endl;
        std::cout << "File appears to be truncated." << std::endl;
        std::cout << "Expected " << except.expected << " bytes" << std::endl;
        std::cout << "    Read " << except.read << " bytes" << std::endl;
        return 0;
    }
    catch (const badCRCException& except)
    {
        std::cout << "Error: Cannot load MDA config file ";
        std::cout << mdaConfigFilename << std::endl;
        std::cout << "The CRC is incorrect." << std::endl;
        std::cout << std::hex;
        std::cout << "Computed CRC is 0x" << except.computed << std::endl;
        std::cout << "  Stored CRC is 0x" << except.stored << std::endl;
        return 0;
    }
    catch (const std::runtime_error& except)
    {
        std::cout << "Error: Cannot load MDA config file ";
        std::cout << mdaConfigFilename << std::endl;
        std::cout << except.what() << std::endl;
        return 0;
    }
    std::cout << "Configuration loaded." << std::endl;


    /*
     *  Open the mdalog file
     * 
     * Overwriting existing files is somewhat user-hostile, so we create a new one
     */
    for (int i = 1; ; i++)
    {
        MDAlogFilepath = MDAlogFilename + std::to_string(i) + MDAlogFileext;
        MDA_log.open(MDAlogFilepath, std::fstream::in);
        if (!MDA_log)
        {
            MDA_log.close();
            MDA_log.open(MDAlogFilepath, std::fstream::out);
            break;
        }
        MDA_log.close();

        if (i > 50)
        {
            std::cout << "Failed to open a new MDA logfile after 50 attempts!" << std::endl;
            std::cout << "Cleanup of the old log files is required!" << std::endl;
            return 0;
        }
    }
    if (!MDA_log.good())
    {
        std::cout << "Failed to open MDA logfile: " << MDAlogFilepath << std::endl;
        MDA_log.close();
        return 0;
    }
    std::cout << "Using MDA logfile: " << MDAlogFilepath << std::endl;
    

    /*
     *  Register signal handlers
     * 
     * It't not safe to simply allow the application to exit on SIGINT,
     * we need to cleanup and park the MB.
     * 
     */
    struct sigaction sig;
    sig.sa_handler = &sig_handler;
    std::cerr << "Registering signal handlers...   ";
    if (sigaction(SIGINT, &sig, nullptr) != 0)
    {
        std::cerr << "Unable to register signal handler!" << std::endl;
        return 0;
    }
    std::cerr << "Done." << std::endl;


    /*
     *  Initialize the Motion Base interface object
     * 
     * It will spawn its own threads and get out of our way while we handle the
     * UI
     */
    std::cout << "Initializing MB interface...   ";
    mbinterface motion_base(MBport, localPort, MBaddr, 
                            XPport, config, MDA_log, subgrav);
    std::cout << "Done." << std::endl;


    /* --- End of init --- */

    /*
     *  At this point, everything should have initialized correctly, 
     *  so we can enter curses mode to present a prettier interface.
     */
    std::cout << "Starting curses..." << std::endl;
    initscr();				/* start the curses mode */
    raw();                  //Disable line buffering
    //keypad(stdscr, TRUE);   //Advanced keyboard stuff
    noecho();               //Don't echo getch
    nodelay(stdscr, TRUE);  //Don't block waiting for input

    auto nextTick = std::chrono::high_resolution_clock::now();
    auto sampleTime = std::chrono::nanoseconds( (int)(1e9 / 60));

    MCISvector sf, angv, att, pos, rot;

    std::stringstream vector_stream;
    char out_str[128];
    
    int consoleInput = 0;
    while(cont)
    {
        nextTick += sampleTime;
        clear();
        //mvprintw(1, 1, "E - Engage     R - Ready     O - Override    P - Park    Q - Exit");
        mvprintw(1, 1, "P - Park     Q - Exit");
        mvprintw(2, 5, "MB state: ");
        switch (motion_base.get_MB_status())
        {
            case MB_STATE_POWER_UP:
                printw("POWER UP ");
                break;
            case MB_STATE_IDLE:
                printw("IDLE     ");
                break;
            case MB_STATE_STANDBY:
                printw("STANDBY  ");
                break;
            case MB_STATE_ENGAGED:
                printw("ENGAGED  ");
                break;
            case MB_STATE_PARKING:
                printw("PARKING  ");
                break;
            case MB_STATE_FAULT1:
                printw("FAULT1   ");
                break;
            case MB_STATE_FAULT2:
                printw("FAULT2   ");
                break;
            case MB_STATE_FAULT3:
                printw("FAULT3   ");
                break;
            case MB_STATE_DISABLED:
                printw("DISABLED ");
                break;
            case MB_STATE_INHIBITED:
                printw("INHIBITED");
                break;
            default:
                printw("UNKNOWN  ");
                break;
        }
        if (subgrav)
        {
            mvprintw(2, 40, "Subtracting gravity");
        }
        else
        {
            mvprintw(2, 40, "NO GRAVITY SUBTRACTION");
        }

        mvprintw(3, 5, "Interface status: ");
        switch (motion_base.get_iface_status())
        {
            case ESTABLISH_COMMS:
                printw("Establishing communication               ");
                break;
            case WAIT_FOR_ENGAGE:
                printw("Waiting for user to engage               ");
                mvprintw(4, 5, "Press E to send MB ENGAGE command.");
                break;
            case ENGAGING:
                printw("MB engaging, please wait...              ");
                break;
            case WAIT_FOR_READY:
                printw("MB engaged. Waiting for user ready signal");
                mvprintw(4, 5, "Press R to initiate motion.");
                break;
            case RATE_LIMITED:
                printw("Engaged - Output is rate limited         ");
                break;
            case ENGAGED:
                printw("Engaged.                                 ");
                break;
            case PARKING:
                printw("MB parking, please wait...               ");
                break;
            case MB_FAULT:
                printw("MB reports a fault! Troubleshooting required!");
                break;
            case MB_RECOVERABLE_FAULT:
                printw("MB reports a possibly recoverable fault! ");
                mvprintw(4, 5, "Press R to send MB RESET command.");
                break;
        }

        motion_base.get_MDA_status(sf, angv, att, pos, rot);

        sf.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(6, 5, "Input acceleration:     %s", out_str);

        angv.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(7, 5, "Input angular velocity: %s", out_str);

        att.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(8, 5, "Input attitude:         %s", out_str);

        pos.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(10, 5, "Output position:        %s", out_str);

        rot.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(11, 5, "Output angles:          %s", out_str);

        mvprintw(15, 5, "Send clock ticks: %d", motion_base.get_ticks());

        refresh();

        consoleInput = getch();
        flushinp();
        if (consoleInput != ERR)
        {
            iface_status status = motion_base.get_iface_status();
            switch (consoleInput)
            {
                case 'e':
                case 'E':
                    if (status == WAIT_FOR_ENGAGE)
                    {
                        motion_base.setEngage();
                    }
                    break;
                case 'r':
                case 'R':
                    if (status == WAIT_FOR_READY)
                    {
                        motion_base.setReady();
                    }
                    else if (status == MB_RECOVERABLE_FAULT)
                    {
                        motion_base.setReset();
                    }
                    break;
                case 'o':
                case 'O':
                    // This option is somewhat dangerous and shouldn't be necessary 
                    // at this point in time.
                    // It manually skips ahead to the "next" state
                    //
                    //motion_base.setOverride();
                    break;
                case 'p':
                case 'P':
                    motion_base.setPark();
                    break;
                case 'q':
                case 'Q':
                case '.':   //Allows for emergency stop using Logitech presentation remotes
                    if ((status == ENGAGING) ||  (status == WAIT_FOR_READY)
                        || (status == ENGAGED) || (status == RATE_LIMITED)
                        || (status == PARKING))
                    {
                        motion_base.setPark();
                    }
                    else
                    {
                        cont = false;
                    }
                    break;
            }
        }
        std::this_thread::sleep_until(nextTick);
    }

    endwin();
    std::cout << "Exited curses mode" << std::endl;

    motion_base.stop();

    std::cout << "Threads should be joining now..." << std::endl;
    
    MDA_log.close();
    
    return 0;


}

void sig_handler(int signo)
{
    cont = false;
}
