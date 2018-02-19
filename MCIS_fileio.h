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

#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include "discreteMath.h"


/*
 *  readMCISinputs
 * 
 * Reads a set of six doubles to serve as inputs for MCIS
 * X,Y,Z acceleration, roll, pitch, yaw rates
 */
bool readMCISinputs(std::istream& infile, MCISvector& accIn, MCISvector& angIn);

/*
 *  writeMCISoutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISoutputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn);

/*
 *  writeBaseMCISoutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 * 
 * Actual heavy lifting goes here
 */
void writeBaseMCISoutputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn);

/*
 *  writeMCISfullOutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISfullOutputs(std::ostream& outfile, 
                        const MCISvector& accIn, 
                        const MCISvector& angIn,
                        const MCISvector& noTCin);