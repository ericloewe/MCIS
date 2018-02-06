/*
 *  Barebones test application for MCIS
 */

#include <iostream>
#include <fstream>
#include "MCIS_config.h"
#include "MCIS_MDA.h"
#include "discreteMath.h"
#include "MCIS_fileio.h"

#define configFileName "MCISconfig.bin"
#define inFilename "MCISinput.csv"
#define outFilename "MCISoutput.csv"


int main ()
{
    MCISconfig config;
    char *fileIn = (char *)&config;

    std::ifstream configFile;
    std::ifstream inputFile;
    std::ofstream outputFile;

    configFile.open(configFileName, std::ios_base::binary);

    if (!configFile.good())
    {
        std::cout << "Error opening config file " << configFileName << std::endl;
        return 0;
    }

    configFile.read(fileIn, sizeof(MCISconfig));
    if (configFile.gcount() != sizeof(MCISconfig))
    {
        std::cout << "Config file truncated! Cannot start MCIS using config file " << configFileName << std::endl;
        return 0;
    }

    std::cout << "Configuration loaded." << std::endl;

    inputFile.open(inFilename);
    outputFile.open(outFilename);

    if (inputFile.good() && outputFile.good())
    {
        std::cout << "Input and output .csv files opened." << std::endl;
    }
    else
    {
        std::cout << "Failed to open files:" << std::endl;
        if (!inputFile.good())
        {
            std::cout << inFilename;
        }
        if (!outputFile.good())
        {
            std::cout << outFilename;
        }
        return 0;
    }
        

    MCIS_MDA mda{config};

    MCISvector sfIn, angIn, posOut, angOut;

    while (readMCISinputs(inputFile, sfIn, angIn))
    {
        mda.nextSample(sfIn, angIn);
        writeMCISfullOutputs(outputFile, mda.getPos(), mda.getangle(), mda.getAngleNoTC());
        outputFile << std::endl;
    }

    /*while (inputFile.good())
    {
        readMCISinputs(inputFile, sfIn, angIn);
        mda.nextSample(sfIn, angIn);
        writeMCISfullOutputs(outputFile, mda.getPos(), mda.getangle(), mda.getAngleNoTC());
    }*/

    //config.print(std::cout);

    /*sfIn.assign(0,0,9.81);
    for (int i = 0; i < 4800; i++)
    {
        if (i == 600)
        {
            sfIn.assign(5,0,9.81);
        }
        else if (i == 1800)
        {
            sfIn.assign(-5,0,9.81);
        }
        else if (i == 3000)
        {
            sfIn.assign(0,0,9.81);
        }
        mda.nextSample(sfIn, angIn);
        writeMCISfullOutputs(outputFile, mda.getPos(), mda.getangle(), mda.getAngleNoTC());
        outputFile << std::endl;
    }*/




    return 0;

}
