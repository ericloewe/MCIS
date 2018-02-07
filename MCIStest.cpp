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

#define testFolder "./testinputs/"
#define testLen 9
#define testStart 1


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

    /*inputFile.open(inFilename);
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
    }*/
        

    std::string path;
    std::ifstream infile;
    std::ofstream outfile;

    for (int i = 1; i <= testLen; i++)
    {
        path = testFolder;
        path += "test";
        path += std::to_string(i);
        path += ".csv";
        std::cout << "Opening file "<< path << "  ... ";

        infile.open(path);
        if (!infile.good())
        {
            std::cout << "Failed!" << std::endl;
        }
        else
        {
            std::cout << "Succeeded!" << std::endl;

            path = testFolder;
            path += "test";
            path += std::to_string(i);
            path += "out";
            path += ".csv";
            outfile.open(path);

            MCIS_MDA mda{config};

            MCISvector sfIn, angIn, posOut, angOut;

            while (readMCISinputs(infile, sfIn, angIn))
            {
                mda.nextSample(sfIn, angIn);
                writeMCISfullOutputs(outfile, mda.getPos(), mda.getangle(), mda.getAngleNoTC());
                outfile << std::endl;
            }
        }
        infile.close();
        outfile.close();
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
