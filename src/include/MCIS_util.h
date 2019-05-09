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

#pragma once

#include <cstdint>

/*
 *  floatNetToHost
 * 
 * Convert a 32-bit float from Network byte order to Host byte order
 * 
 */
float floatNetToHost(uint32_t netFloat);

/*
 *  floatLEToHost
 * 
 * Convert a 32-bit float from little-endian to Host byte order
 * 
 */
float floatLEToHost(uint32_t LEFloat);

/*
 *  floatHostToNet
 * 
 * Convert a 32-bit float from Network byte order to Host byte order
 * 
 */
uint32_t floatHostToNet(float hostFloat);

/*
 *  doubleNetToHost
 * 
 *  Convert a 64-bit double from Network byte order to Host byte order
 *
 */
double doubleNetToHost(double netDouble);

/*
 *  doubleNetToHost
 * 
 *  Convert a 64-bit double from Network byte order to Host byte order
 *
 */
double doubleNetToHost(uint64_t netDouble);