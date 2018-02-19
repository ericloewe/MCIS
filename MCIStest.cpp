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
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

/*
 *  Barebones test application for MCIS
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "MCIS_config.h"
#include "MCIS_MDA.h"
#include "discreteMath.h"
#include "MCIS_fileio.h"

#define configFileName "MCISconfig.bin"
#define inFilename "MCISinput.csv"
#define outFilename "MCISoutput.csv"

#define testFolder "./testinputs/"
#define testLen 9
#define testStart 1


int main (int argc, char **argv)
{
    MCISconfig config;
    char *fileIn = (char *)&config;

    std::ifstream configFile;
    std::ifstream inputFile;
    std::ofstream outputFile;

    //Check if we have enough arguments to do anything
    if (argc < 2)
    {
        std::cout << "Usage: MCIStest input_file" << std::endl;
        return 0;
    }

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

    std::string path;
    std::ifstream infile;
    std::ofstream outfile;

    for (int i = 1; i < argc; i++)
    {
        infile.open(argv[i]);
        if (!infile.good())
        {
            std::cout << "Failed to open file: " << argv[i] << std::endl;
            infile.close();
            continue;
        }
    
        std::cout << "Reading file: " << argv[i] << "  ... ";
        path  = argv[i];
        path += "out.csv";
        outfile.open(path);

        if (!outfile.good())
        {
            std::cout << "Failed to open output file: " << path << std::endl;
            infile.close();
            outfile.close();
            continue;
        }

        MCIS_MDA mda{config};

        MCISvector sfIn, angIn, posOut, angOut;

        while (readMCISinputs(infile, sfIn, angIn))
        {
            mda.nextSample(sfIn, angIn);
            writeMCISfullOutputs(outfile, mda.getPos(), mda.getangle(), mda.getAngleNoTC());
        }

        std::cout << "done." << std::endl;

        infile.close();
        outfile.close();
    }
    
    return 0;

}
