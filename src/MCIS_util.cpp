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

/*
 * Utilities for MCIS
 */

#include <cstdint>
#include <arpa/inet.h>
#include "include/MCIS_util.h"

/*
 *  floatNetToHost
 * 
 * Convert a 32-bit float from Network byte order to Host byte order
 * 
 */
float floatNetToHost(uint32_t bigEndianFloat)
{
    /*uint8_t value[4] = {};
    memcpy(&value, &bigEndianFloat, 4);

    uint32_t hostVal = ((uint32_t) value[3] << 0)
                    |  ((uint32_t) value[2] << 8)
                    |  ((uint32_t) value[1] << 16)
                    |  ((uint32_t) value[0] << 24);*/
    uint32_t hostVal = ntohl(bigEndianFloat);
    float *convFloat = reinterpret_cast<float *>(hostVal);
    return *convFloat;
}

/*
 *  floatLEToHost
 * 
 * Convert a 32-bit float from little-endian to Host byte order
 * 
 */
float floatLEToHost(uint32_t LEFloat)
{
    uint8_t *inBuf = reinterpret_cast<uint8_t *>(&LEFloat);

    /*
     *  Seems messy and inefficient, but most compilers will boil it down to no-op
     *  for big-endian systems and bswap32 for little-endian systems
     */
    uint64_t hostFloat;
    hostFloat = ((uint64_t)inBuf[0] << 0)  | 
                ((uint64_t)inBuf[1] << 8)  | 
                ((uint64_t)inBuf[2] << 16) | 
                ((uint64_t)inBuf[3] << 24);

    return *reinterpret_cast<double *>(&hostFloat);
}

/*
 *  floatHostToNet
 * 
 * Convert a 32-bit float from Network byte order to Host byte order
 * 
 */
uint32_t floatHostToNet(float hostFloat)
{
    uint32_t hostVal = *(reinterpret_cast<uint32_t *>(&hostFloat));
    return htonl(hostVal);
}

/*
 *  doubleNetToHost
 * 
 *  Convert a 64-bit double from Network byte order to Host byte order
 *
 */
double doubleNetToHost(double netDouble)
{
    uint8_t *inBuf = reinterpret_cast<uint8_t *>(&netDouble);

    /*
     *  Seems messy and inefficient, but most compilers will boil it down to no-op
     *  for big-endian systems and bswap64 for little-endian systems
     */
    uint64_t hostDouble;
    hostDouble =    ((uint64_t)inBuf[7] << 0)  | 
                    ((uint64_t)inBuf[6] << 8)  | 
                    ((uint64_t)inBuf[5] << 16) | 
                    ((uint64_t)inBuf[4] << 24) | 
                    ((uint64_t)inBuf[3] << 32) | 
                    ((uint64_t)inBuf[2] << 40) | 
                    ((uint64_t)inBuf[1] << 48) | 
                    ((uint64_t)inBuf[0] << 56);

    return *reinterpret_cast<double *>(&hostDouble);
}

/*
 *  doubleNetToHost
 * 
 *  Convert a 64-bit double from Network byte order to Host byte order
 *
 */
double doubleNetToHost(uint64_t netDouble)
{
    uint8_t *inBuf = reinterpret_cast<uint8_t *>(&netDouble);

    /*
     *  Seems messy and inefficient, but most compilers will boil it down to no-op
     *  for big-endian systems and bswap64 for little-endian systems
     */
    uint64_t hostDouble;
    hostDouble =    ((uint64_t)inBuf[7] << 0)  | 
                    ((uint64_t)inBuf[6] << 8)  | 
                    ((uint64_t)inBuf[5] << 16) | 
                    ((uint64_t)inBuf[4] << 24) | 
                    ((uint64_t)inBuf[3] << 32) | 
                    ((uint64_t)inBuf[2] << 40) | 
                    ((uint64_t)inBuf[1] << 48) | 
                    ((uint64_t)inBuf[0] << 56);

    return *reinterpret_cast<double *>(&hostDouble);
}