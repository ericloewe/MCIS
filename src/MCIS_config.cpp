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
#include <cstdint>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include "include/MCIS_config.h" 
#include "include/MCIS_util.h"
extern "C" {
#include "include/crc.h"
}
/*
 *  Constructors
 */

//Default constructor - does nothing
MCISconfig::MCISconfig()
{}

//Initialization constructor - loads and verifies a config file
MCISconfig::MCISconfig(std::string filename)
{
    load(filename);
    /*if (!load(filename))
    {
        std::runtime_error except(
            "Failed to load MDA config\n");
        throw except;
    }*/
}

/*
 *  Other functions
 */

/*
 *  load a config file into this object
 */
void MCISconfig::load(const std::string& filename)
{
    uint8_t buffer[sizeof(MCISconfig)];

    FILE *infileFD = fopen(filename.c_str(), "rb");

    //We need to ensure the file is okay
    //First step is to see if we can even read from it
    if (infileFD == NULL)
    {
        std::runtime_error except(
            "Failed to open MDA config file\n");
        throw except;
    }

    //Second, read in the file and check if the size is right
    int bytes = fread((void *)&buffer, 1, sizeof(MCISconfig), infileFD);
    if (bytes != sizeof(MCISconfig))
    {
        badLengthException except(sizeof(MCISconfig), bytes);
        throw except;
    }
    fclose(infileFD);

    //Third, run the CRC and check it against the stored one
    crc inFileCRC = crcSlow((unsigned char *)buffer, CRC_POSITION);
    crc storedCRC;
    memcpy((void *)&storedCRC, (void *)(buffer + CRC_POSITION), sizeof(uint32_t));
    storedCRC = ntohl(storedCRC);   //Handle endianness if necessary
    if (storedCRC != inFileCRC)
    {
        badCRCException except(storedCRC, inFileCRC);
        throw except;
    }

    //Fourth and finally, make sure the version is supported
    //For now, we only support version 5
    char fileHeader[16];
    memcpy((void *)fileHeader, (void *)(buffer), 16);
    if (strncasecmp(fileHeader, "MCIS v05 CONFIG ", 16) != 0)
    {
        switch (fileHeader[7])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            {
                //These are all old, little-endian versions
                //that are no longer supported.
                littleEndianConfigException except(
                    "MDA config uses old little-endian format. v05 is required.\n");
                throw except;
            }
            default:
            {
                unsupportedConfigTypeException except(
                    "Unknown MDA config format or invalid header.\n");
                throw except;
            }
        }
    }

    //Now we can read all the stuff in.
    //We cheese things a bit by casting a pointer to the buffer to MCISconfig*
    //This avoids keeping track of offsets manually
    MCISconfig *readConfig = (MCISconfig *)buffer;

    //memcpy the header over for the sake of completeness
    memcpy((void *)this -> configHeader, readConfig -> configHeader, 28);

    this -> sampleRate = ntohl(readConfig -> sampleRate);

    this -> K_SF_x = doubleNetToHost(readConfig -> K_SF_x);
    this -> K_SF_y = doubleNetToHost(readConfig -> K_SF_y);
    this -> K_SF_z = doubleNetToHost(readConfig -> K_SF_z);

    this -> K_p = doubleNetToHost(readConfig -> K_p);
    this -> K_q = doubleNetToHost(readConfig -> K_q);
    this -> K_r = doubleNetToHost(readConfig -> K_r);

    this -> lim_SF_x = doubleNetToHost(readConfig -> lim_SF_x);
    this -> lim_SF_y = doubleNetToHost(readConfig -> lim_SF_y);
    this -> lim_SF_z = doubleNetToHost(readConfig -> lim_SF_z);

    this -> lim_p = doubleNetToHost(readConfig -> lim_p);
    this -> lim_q = doubleNetToHost(readConfig -> lim_q);
    this -> lim_r = doubleNetToHost(readConfig -> lim_r);

    this -> K_TC_x = doubleNetToHost(readConfig -> K_TC_x);
    this -> K_TC_y = doubleNetToHost(readConfig -> K_TC_y);

    this -> lim_TC_x = doubleNetToHost(readConfig -> lim_TC_x);
    this -> lim_TC_y = doubleNetToHost(readConfig -> lim_TC_y);

    this -> ratelim_TC_x = doubleNetToHost(readConfig -> ratelim_TC_x);
    this -> ratelim_TC_y = doubleNetToHost(readConfig -> ratelim_TC_y);

    //Now come the filters. Thankfully, there are functions for that
    loadContinuousFiltParams(&this -> filt_SF_HP_x_cont, &readConfig -> filt_SF_HP_x_cont);
    loadDiscreteFiltParams(&this -> filt_SF_HP_x_disc, &readConfig -> filt_SF_HP_x_disc);

    loadContinuousFiltParams(&this -> filt_SF_HP_y_cont, &readConfig -> filt_SF_HP_y_cont);
    loadDiscreteFiltParams(&this -> filt_SF_HP_y_disc, &readConfig -> filt_SF_HP_y_disc);

    loadContinuousFiltParams(&this -> filt_SF_HP_z_cont, &readConfig -> filt_SF_HP_z_cont);
    loadDiscreteFiltParams(&this -> filt_SF_HP_z_disc, &readConfig -> filt_SF_HP_z_disc);

    loadContinuousFiltParams(&this -> filt_SF_LP_x_cont, &readConfig -> filt_SF_LP_x_cont);
    loadDiscreteFiltParams(&this -> filt_SF_LP_x_disc, &readConfig -> filt_SF_LP_x_disc);

    loadContinuousFiltParams(&this -> filt_SF_LP_y_cont, &readConfig -> filt_SF_LP_y_cont);
    loadDiscreteFiltParams(&this -> filt_SF_LP_y_disc, &readConfig -> filt_SF_LP_y_disc);

    loadContinuousFiltParams(&this -> filt_p_HP_cont, &readConfig -> filt_p_HP_cont);
    loadDiscreteFiltParams(&this -> filt_p_HP_disc, &readConfig -> filt_p_HP_disc);

    loadContinuousFiltParams(&this -> filt_q_HP_cont, &readConfig -> filt_q_HP_cont);
    loadDiscreteFiltParams(&this -> filt_q_HP_disc, &readConfig -> filt_q_HP_disc);

    loadContinuousFiltParams(&this -> filt_r_HP_cont, &readConfig -> filt_r_HP_cont);
    loadDiscreteFiltParams(&this -> filt_r_HP_disc, &readConfig -> filt_r_HP_disc);

    //We copy the CRC just for the sake of completeness
    this -> CRC = ntohl(readConfig -> CRC);

    //And finally we memcpy the comments
    memcpy((void *)this -> comments, (void *)readConfig -> comments, 1100);
}

/*
 *  loadContinuousFiltParams
 * 
 * Loads a continuous-time filter parameters structure, automagically converting 
 * endianness if needed. 
 */
void loadContinuousFiltParams(continuousFiltParams *filt, continuousFiltParams *inbuf)
{
    filt -> filtOrder = inbuf -> filtOrder; //Single byte, not endian

    memcpy((void *)filt -> description, (void *)inbuf -> description, 15);

    for (int i = 0; i < 8; i++)
    {
        filt -> b[i] = doubleNetToHost(inbuf -> b[i]);
        filt -> a[i] = doubleNetToHost(inbuf -> a[i]);
    }
}

/*
 *  loadDiscreteFiltParams
 * 
 * Loads a discrete-time filter parameters structure, automagically converting 
 * endianness if needed. 
 */
void loadDiscreteFiltParams(discreteFiltParams *filt,  discreteFiltParams *inbuf)
{
    filt -> sectionsInUse = inbuf -> sectionsInUse; //Single byte, not endian

    memcpy((void *)filt -> description, (void *)inbuf -> description, 15);

    for (int i = 0; i < 4; i++)
    {
        loadDiscreteBiquadParams(&filt -> biquads[i], &inbuf -> biquads[i]);
    }
}

/*
 *  loadDiscreteBiquadParams
 * 
 * Loads a discrete-time filter biquad section parameters structure, automagically converting 
 * endianness if needed. 
 */
void loadDiscreteBiquadParams(discreteBiquadSectionParams *sect, discreteBiquadSectionParams *inbuf)
{
    sect -> b0 = doubleNetToHost(inbuf -> b0);
    sect -> b1 = doubleNetToHost(inbuf -> b1);
    sect -> b2 = doubleNetToHost(inbuf -> b2);

    sect -> a1 = doubleNetToHost(inbuf -> a1);
    sect -> a2 = doubleNetToHost(inbuf -> a2);

    sect -> gain = doubleNetToHost(inbuf -> gain); 
}

/*
 *  Printer functions
 */
void MCISconfig::print(std::ostream& dest)
{
    dest << "Printing MCIS configuration file:" << std::endl;

    dest << configHeader << std::endl;
    dest << "Sample rate [Hz]: " << sampleRate << std::endl;

    dest << "High-pass Specific Force gains (X, Y, Z):" << std::endl;
    dest << K_SF_x << "  |  " << K_SF_y << "  |  " << K_SF_z << std::endl;

    dest << "High-pass Roll Rate gains (p, q, r):" << std::endl;
    dest << K_p << "  |  " << K_q << "  |  " << K_r << std::endl;

    dest << "High-pass Specific Force limits (X, Y, Z) [m/s^2]:" << std::endl;
    dest << lim_SF_x << "  |  " << lim_SF_y << "  |  " << lim_SF_z << std::endl;

    dest << "High-pass Roll Rate limits (p, q, r) [rad/s]:" << std::endl;
    dest << lim_p << "  |  " << lim_q << "  |  " << lim_r << std::endl;

    dest << "Tilt coordination gains (x,y):" << std::endl;
    dest << K_TC_x << "  |  " << K_TC_y << std::endl;

    dest << "Tilt coordination limits (x,y) [m/s^2]:" << std::endl;
    dest << lim_TC_x << "  |  " << lim_TC_y << std::endl;

    dest << "Tilt coordination rate limits (x,y) [rad/s]:" << std::endl;
    dest << ratelim_TC_x << "  |  " << ratelim_TC_y << std::endl;

    dest << std::endl << "Filter definitions:" << std::endl;
    filt_SF_HP_x_cont.print(dest);
    filt_SF_HP_x_disc.print(dest);
    
    filt_SF_HP_y_cont.print(dest);
    filt_SF_HP_y_disc.print(dest);
    
    filt_SF_HP_z_cont.print(dest);
    filt_SF_HP_z_disc.print(dest);
    
    
    filt_SF_LP_x_cont.print(dest);
    filt_SF_LP_x_disc.print(dest);

    filt_SF_LP_y_cont.print(dest);
    filt_SF_LP_y_disc.print(dest);


    filt_p_HP_cont.print(dest);
    filt_p_HP_disc.print(dest);

    filt_q_HP_cont.print(dest);
    filt_q_HP_disc.print(dest);

    filt_r_HP_cont.print(dest);
    filt_r_HP_disc.print(dest);
}

void continuousFiltParams::print(std::ostream& dest)
{
    int temp = filtOrder;
    dest << std::endl << "Continuous-time filter of order " << temp << std::endl;
    dest << "Filter header (optional): " << std::endl;
    dest << description << std::endl;

    dest << "Numerator gains (s^7 to s^0): " << std::endl;
    for (int i = 0; i < 8; i++)
    {
        dest << b[i] << std::endl;
    }
    dest << "Denominator gains (s^7 to s^0): " << std::endl;
    for (int i = 0; i < 8; i++)
    {
        dest << a[i] << std::endl;
    }
}

void discreteFiltParams::print(std::ostream& dest)
{
    int temp = sectionsInUse;
    dest << std::endl << "Discrete-time filter using " << temp << " biquad sections" << std::endl;
    dest << "Filter header (optional): " << std::endl;
    dest << description << std::endl;

    dest << "Biquadratic section parameters (including those not in use):" << std::endl;
    for (int i = 0; i < 4; i++)
    {
        biquads[i].print(dest);
    }
}

void discreteBiquadSectionParams::print(std::ostream& dest)
{
    dest << "Biquadratic section parameters:" << std::endl;
    dest << "b0, b1, b2:  " << b0 << "   " << b1 << "   " << b2 << std::endl;
    dest << "a0, a1, a2:  " << 1.0 << "   " << a1  << "   " << a2 << std::endl;
    dest << "Gain:  " << gain << std::endl;
}

badCRCException::badCRCException(uint32_t storedCRC, uint32_t computedCRC) :
    std::runtime_error("Bad CRC\n"),
    stored{storedCRC},
    computed{computedCRC}
{}

badLengthException::badLengthException(unsigned int expectedLen, unsigned int readLen) :
    std::length_error("Bad length\n"),
    expected{expectedLen},
    read{readLen}
{}