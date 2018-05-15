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

/* All this stuff is going into a config file */
//#define MB_IP 0x807F3778 //Old IP, 128.127.55.120
#define MB_IP 0xC0A81405
#define MB_PORT 991    //CHANGE ME!
#define LOCAL_PORT 10500 //CHANGE ME!
#define XPLANE_RECV_PORT 49000 //CHANGE ME!
#define configFileName "MCISconfig.bin"
#define MDA_LOGNAME "mdalog.csv"
#define XP_RECV_LOGNAME "xplog.csv"
#define MB_LOGNAME "mblog.csv"

bool cont = true;

void sig_handler(int signo);

int main()
{
    MCISconfig config;
    char *fileIn = (char *)&config;
    std::ifstream configFile;
    std::fstream MDA_log;
    
    //TODO - Proper config treatment
    //Try to open the config file...
    configFile.open(configFileName, std::ios_base::binary);
    if (!configFile.good())
    {
        std::cout << "Error opening config file " << configFileName << std::endl;
        return 0;
    }
    //And try to read it.
    configFile.read(fileIn, sizeof(MCISconfig));
    if (configFile.gcount() != sizeof(MCISconfig))
    {
        std::cout << "Config file truncated! Cannot start MCIS using config file " << configFileName << std::endl;
        return 0;
    }
    std::cout << "Configuration loaded." << std::endl;

    //Open MDA log
    MDA_log.open(MDA_LOGNAME, MDA_log.out | MDA_log.trunc);
    if (!MDA_log.good())
    {
        std::cout << "Failed to open MDA logfile: " << MDA_LOGNAME << std::endl;
        MDA_log.close();
        return 0;
    }
    
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
                            XPLANE_RECV_PORT, config, MDA_log);
    std::cout << " Done." << std::endl;
    std::cout << "Starting curses..." << std::endl;

    initscr();				/* start the curses mode */
    raw();                  //Disable line buffering
    //keypad(stdscr, TRUE);   //Advanced keyboard stuff
    //noecho();               //Don't echo getch
    nodelay(stdscr, TRUE);  //Don't block waiting for input

    auto nextTick = std::chrono::high_resolution_clock::now();
    auto sampleTime = std::chrono::nanoseconds( (int)(1e9 / 60));

    MCISvector sf, angv, pos, rot;

    std::stringstream vector_stream;
    char out_str[128];
    
    

    int consoleInput = 0;
    while(cont)
    {
        nextTick += sampleTime;
        //clear();
        mvprintw(1, 1, "E - Engage     R - Ready     O - Override    P - Park    Q - Exit");
        mvprintw(2, 5, "MB state: ");
        switch (motion_base.get_MB_status())
        {
            case MB_STATE_POWER_UP:
                printw("POWER UP");
                break;
            case MB_STATE_IDLE:
                printw("IDLE");
                break;
            case MB_STATE_STANDBY:
                printw("STANDBY");
                break;
            case MB_STATE_ENGAGED:
                printw("ENGAGED");
                break;
            case MB_STATE_PARKING:
                printw("PARKING");
                break;
            case MB_STATE_FAULT1:
                printw("FAULT1");
                break;
            case MB_STATE_FAULT2:
                printw("FAULT2");
                break;
            case MB_STATE_FAULT3:
                printw("FAULT3");
                break;
            case MB_STATE_DISABLED:
                printw("DISABLED");
                break;
            case MB_STATE_INHIBITED:
                printw("INHIBITED");
                break;
            default:
                printw("UNKNOWN");
                break;
        }

        mvprintw(3, 5, "Interface status: %d", motion_base.get_iface_status());

        motion_base.get_MDA_status(sf, angv, pos, rot);

        sf.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(5, 5, "Input acceleration:    %s", out_str);

        angv.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(6, 5, "Input angular velocity: %s", out_str);

        pos.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(9, 5, "Output position:        %s", out_str);

        rot.print(vector_stream);
        vector_stream.getline(out_str, 128);
        mvprintw(10, 5, "Output angles:          %s", out_str);

        mvprintw(14, 5, "Send clock ticks: %d", motion_base.get_ticks());

        refresh();

        consoleInput = getch();
        flushinp();
        if (consoleInput != ERR)
        {
            switch (consoleInput)
            {
                case 'e':
                case 'E':
                    motion_base.setEngage();
                    break;
                case 'r':
                case 'R':
                    motion_base.setReady();
                    break;
                case 'o':
                case 'O':
                    motion_base.setOverride();
                    break;
                case 'p':
                case 'P':
                    motion_base.setPark();
                    break;
                case 'q':
                case 'Q':
                    iface_status status = motion_base.get_iface_status();
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
    
    configFile.close();
    MDA_log.close();
    
    return 0;


}

void sig_handler(int signo)
{
    cont = false;
}
