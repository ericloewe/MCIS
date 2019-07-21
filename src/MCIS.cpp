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
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include "include/MCIS_MB_interface.h"
#include "include/MCIS_config.h"

/* All this stuff is going into a config file */
//#define MB_IP 0x807F3778 //Old IP, 128.127.55.120
#define MB_IP 0xC0A81405  //192.168.20.5 in hex
#define MB_PORT 991    //CHANGE ME!
#define LOCAL_PORT 10500 //CHANGE ME!
#define XPLANE_RECV_PORT 49000 //CHANGE ME!
#define configFileName "MCISconfig.bin"
#define MDA_LOGNAME "mdalog"
#define MDA_LOGEXT  ".csv"
#define XP_RECV_LOGNAME "xplog.csv"
#define MB_LOGNAME "mblog.csv"

#define NO_GRAV_OPNAME "-nograv"

bool cont = true;

void sig_handler(int signo);

int main(int argc, char *argv[])
{
    MCISconfig config;
    std::fstream MDA_log;
    bool subgrav = true;

    if (argc > 1)
    {
        std::string option = argv[1];
        if (!option.compare(NO_GRAV_OPNAME))
        {
            subgrav = false;
        }
    }
    
    config.load(configFileName);

    std::cout << "Configuration loaded." << std::endl;

    /*
     *  Open the mdalog file
     * 
     * Overwriting existing files is somewhat user-hostile, so we create a new one
     */
    std::string filename = MDA_LOGNAME;
    std::string fileext  = MDA_LOGEXT;
    std::string filepath;
    for (int i = 1; ; i++)
    {
        filepath = filename + std::to_string(i) + fileext;
        MDA_log.open(filepath, std::fstream::in);
        if (!MDA_log)
        {
            MDA_log.close();
            MDA_log.open(filepath, std::fstream::out);
            break;
        }
        MDA_log.close();

        if (i > 50)
        {
            std::cout << "Failed to open a new MDA logfile after 50 attempts!" << std::endl;
            return 0;
        }
    }
    
    
    if (!MDA_log.good())
    {
        std::cout << "Failed to open MDA logfile: " << filepath << std::endl;
        MDA_log.close();
        return 0;
    }

    std::cout << "Using MDA logfile: " << filepath << std::endl;
    
    struct sigaction sig;

    sig.sa_handler = &sig_handler;
    std::cerr << "Registering signal handlers...   ";
    if (sigaction(SIGINT, &sig, nullptr) != 0)
    {
        std::cerr << "Unable to register signal handler!" << std::endl;
        return 0;
    }
    
    std::cerr << "Done." << std::endl;

    std::cout << "Initiating MB interface...";
    mbinterface motion_base(MB_PORT, LOCAL_PORT, MB_IP, 
                            XPLANE_RECV_PORT, config, MDA_log, subgrav);
    std::cout << " Done." << std::endl;
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
