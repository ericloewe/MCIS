


#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include "MCIS_config.h"
#include "MCIS_MDA.h"
#include "discreteMath.h"
#include "MCIS_xplane_sock.h"
#include "MCIS_fileio.h"

#define configFileName "MCISconfig.bin"
#define outFilename "nettest.csv"
#define inputsLogname "nettestinputs.csv"
#define inPort 10555

int main()
{
    MCISconfig config;

    std::ofstream outfile;
    std::ofstream inputslog;
    std::ifstream configFile;
    char *fileIn = (char *)&config;
    
    std::cout << "MCIS network test" << std::endl;

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

    MCIS_MDA mda{config};

    outfile.open(outFilename);
    if (!outfile.good())
    {
        std::cout << "Failed to open output file: " << outFilename << std::endl;
        return 0;
    }

    inputslog.open(inputsLogname);
    if (!inputslog.good())
    {
        std::cout << "Failed to open inputs log file: " << inputsLogname << std::endl;
        return 0;
    }

    MCISvector sfIn, angIn, posOut, angOut;

    xplaneSocket inSock(inPort, XP9);

    auto nextTick = std::chrono::high_resolution_clock::now();
    //std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));
    auto sampleTime = std::chrono::nanoseconds( (int)(1e9 / config.sampleRate));
    //auto sampleTime = std::chrono::microseconds((unsigned long)(10e9));

    std::cout << "std::chrono::duration = " << sampleTime.count() << " ns" << std::endl;
    std::cout << "                      = " << sampleTime.count() * 1e-9 << " s" << std::endl;
    std::cout << "sampleRate = " << config.sampleRate << std::endl;

    //for (int i = 0; i < 120; i++)
    while (true)
    {
        nextTick += sampleTime;

        inSock.getData(sfIn, angIn);
        writeMCISoutputs(inputslog, sfIn, angIn);

        mda.nextSample(sfIn, angIn);
        writeMCISfullOutputs(outfile, mda.getPos(), mda.getangle(), mda.getAngleNoTC());


        std::this_thread::sleep_until(nextTick);
    }


}