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
#include <unistd.h>
#include "MCIS_MB_interface.h"

#define MB_IP 0x807F3778 //CHANGE ME!
#define MB_PORT 991    //CHANGE ME!
#define LOCAL_PORT 992 //CHANGE ME!
#define XPLANE_RECV_PORT 990 //CHANGE ME!
#define configFileName "MCISconfig.bin"

int main()
{
    MCISconfig config;
    char *fileIn = (char *)&config;
    std::ifstream configFile;
    
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


    
    mbinterface motion_base(MB_PORT, LOCAL_PORT, MB_IP, XPLANE_RECV_PORT, config);

    int consoleInput = 0;
    while(true)
    {
        std::cout << "1 - Engage     4 - Ready     7 - Override    0 - Park" << std::endl;
        std::cin >> consoleInput;
        switch (consoleInput)
        {
            case 1:
                motion_base.setEngage();
                break;
            case 4:
                motion_base.setReady();
                break;
            case 7:
                motion_base.setOverride();
                break;
            case 0:
                motion_base.setPark();
                break;
        }



    };
    //The basics are now in place
}