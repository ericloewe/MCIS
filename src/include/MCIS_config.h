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

#pragma once

#include <cstdint>
#include <iostream>
#include <istream>

#define CRC_POSITION 0xbb0

/*
 *  Note on endianness
 * 
 *  On-disk and for interchange purposes, NETWORK BYTE ORDER (BIG-ENDIAN) 
 *  IS USED.
 */

/*
 *  discreteBiquadSectionParams
 * 
 * Transparent struct to keep track of MCIS configuration data
 * 
 * discreteBiquadSection keeps track of the gains used in a discrete-time
 * direct-form II biquad section, in addition to a single gain, which is
 * used, in practice, to keep track of the gain placed at the end of the 
 * series of biquad sections.
 * 
 * sizeof(discreteBiquadSectionParams) should be 48 bytes
 */
class discreteBiquadSectionParams
{
    public:

    double b0, b1, b2;
    double a1, a2;  //a0 is always 1 and is omitted
    double gain;

    void print(std::ostream& destination);
};

/*
 *  discreteFiltParams
 * 
 * Transparent struct to keep track of MCIS configuration data
 * 
 * discreteFiltParams keeps track of everything needed for a digital
 * (discrete-time) filter made up of up to four biquad sections
 * (so, order 8)
 * 
 * sizeof(discreteFiltParams) should be 208 bytes.
 */
class discreteFiltParams
{
    public:

    char sectionsInUse; //Number of biquad sections in use
    char description[15];

    //Biquad sections are used from 0 to 3, in order
    //Sections not in use should be ignored
    discreteBiquadSectionParams biquads[4];

    void print(std::ostream& destination);
};

/*
 *  continuousFiltParams
 * 
 * Transparent struct to keep track of MCIS configuration data
 * 
 * continuousFiltParams keeps track of everything needed for an analog
 * (continuous-time) filter, with transfer function up to
 * 
 *      b0*s^7 + b1*s^6 + ... + b6*s^1 + b7
 *      -----------------------------------
 *      a0*s^7 + a1*s^6 + ... + a6*s^1 + a7
 * 
 * sizeof(continuousFiltParams) should be 144 bytes.
 */
class continuousFiltParams
{
    public:

    char filtOrder; //Order of this filter
    char description[15];

    double b[8];
    double a[8];

    void print(std::ostream& destination);
};

/*
 *  MCISconfig
 * 
 * Transparent struct to keep track of MCIS configuration data
 * 
 * This is the overarching struct that keeps track of an entire set of 
 * parameters, including analog and digital filters for all eight filters.
 */
class MCISconfig
{
    public:
    
    /*
     * C-string header. Should be as follows:
     * 
     * "MCIS vVV config YYYY-MM-DD \0" (28 chars)
     * 
     * VV is the version. Current version is v5, so "v05" (note the zero)
     * YYYY-MM-DD is the date of generation in ISO 8601 format;
     */
    char configHeader[28];
    uint32_t sampleRate; //Sample rate in Hz

    /*
     *  sizeof(Header) + sizeof(sampleRate) = 32 bytes
     */

    //High-pass filtering gains
    double K_SF_x, K_SF_y, K_SF_z, K_p, K_q, K_r;
    //High-pass filtering pre-limiting
    double lim_SF_x, lim_SF_y, lim_SF_z, lim_p, lim_q, lim_r;
    //Low-pass filtering gains
    double K_TC_x, K_TC_y;
    //Low-pass filtering pre-limiting
    double lim_TC_x, lim_TC_y;
    //Low-pass filtering rate limiting
    /*
     *  These are stored in rad/s. The limiters must determine the correct 
     *  per sample rate limit to use based on the current sample rate.
     */
    double ratelim_TC_x, ratelim_TC_y;

    /*
     *  sizeof(this section) = 144 bytes.
     * 
     * Running total: 176 bytes.
     */
    
    //x Specific Force High-Pass filters
    continuousFiltParams    filt_SF_HP_x_cont;
    discreteFiltParams      filt_SF_HP_x_disc;

    //y Specific Force High-Pass filters
    continuousFiltParams    filt_SF_HP_y_cont;
    discreteFiltParams      filt_SF_HP_y_disc;

    //z Specific Force High-Pass filters
    continuousFiltParams    filt_SF_HP_z_cont;
    discreteFiltParams      filt_SF_HP_z_disc;

    //x Specific Force Low-Pass filters
    continuousFiltParams    filt_SF_LP_x_cont;
    discreteFiltParams      filt_SF_LP_x_disc;

    //y Specific Force Low-Pass filters
    continuousFiltParams    filt_SF_LP_y_cont;
    discreteFiltParams      filt_SF_LP_y_disc;

    //Roll rate High-Pass filters
    continuousFiltParams    filt_p_HP_cont;
    discreteFiltParams      filt_p_HP_disc;

    //Pitch rate High-Pass filters
    continuousFiltParams    filt_q_HP_cont;
    discreteFiltParams      filt_q_HP_disc;

    //Yaw rate High-Pass filters
    continuousFiltParams    filt_r_HP_cont;
    discreteFiltParams      filt_r_HP_disc;

    /*
     *  sizeof(Each block) = 352 bytes
     * Running total: 2992 bytes.
     */

    /*
     *  CRC32
     * 
     * We store a CRC32 to help ensure the file isn't corrupted.
     * This is necessary because an out-of-control Motion Base could cause
     * serious injuries and someone might be not using ZFS.
     * 
     * The CRC will *not* protect against malicious actors as it is vulnerable
     * to collisions.
     */
    uint32_t CRC;

    /*
     *  sizeof(CRC) = 4
     * Running Total: 2996 bytes
     */

    /*
     *  We add some padding to get a neat 4kB struct.
     * 
     * It can be used for comments. 1100 chars should be enough 
     * for most comments.
     */
    char comments[1100];

    /*
     *  Total size: 4096 bytes.
     */


    /*
     *  Function declarations
     */
    MCISconfig();
    MCISconfig(std::string filename);

    void load(const std::string& filename);

    void print(std::ostream& destination);

    private:
    //These functions are private because they do not check if the buffer they're given is good.

    
   
};

void loadContinuousFiltParams(continuousFiltParams *filt, continuousFiltParams *inbuf);
void loadDiscreteFiltParams(discreteFiltParams *filt,  discreteFiltParams *inbuf);
void loadDiscreteBiquadParams(discreteBiquadSectionParams *sect, discreteBiquadSectionParams *inbuf);

// Custom exceptions to signal specific errors when loading config files:
class unsupportedConfigTypeException : public std::runtime_error
{
    public:    
    using std::runtime_error::runtime_error;
};

class oldConfigTypeException : public unsupportedConfigTypeException
{
    public:
    using unsupportedConfigTypeException::unsupportedConfigTypeException;
};

class littleEndianConfigException : public oldConfigTypeException
{
    public:
    using oldConfigTypeException::oldConfigTypeException;
};
class badCRCException : public std::runtime_error
{
    public:    
    badCRCException(uint32_t storedCRC, uint32_t computedCRC);
    uint32_t stored, computed;
};
class badLengthException : public std::length_error
{
    public:    
    badLengthException(unsigned int expectedLen, unsigned int readLen);
    unsigned int expected, read;
};

