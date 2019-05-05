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
#include <string>
#include <cstring> //For memcpy
#include <sstream>
#include <exception>
#include "include/discreteMath.h"
#include "include/MCIS_fileio.h"



/*
 *  readMCISinputs
 * 
 * Reads a set of nine doubles to serve as inputs for MCIS
 * X,Y,Z acceleration, roll, pitch, yaw rates, roll, pitch, yaw angles
 * 
 * Returns true on success and false on failure
 */
bool readMCISinputs(std::istream& infile, MCISvector& accIn, MCISvector& angvIn, MCISvector& angIn)
{
    std::string stringbuffer;

    int i;
    double buffer[] = {0,0,0,0,0,0,0,0,0};

    /*
     *  There are 9 elements, yet we stop at 8, not 9. This is because
     * getline(), when given a specific delimiter to work with, will *ignore*
     * \n. This breaks things.
     * 
     * So, we need to read the last element of the row separately.
     * 
     * Yes, this is a pain in the ass. Yes, it's probably easier with the 
     * C standard library. *sigh*
     */
    for (i = 0; i < 8; i++)
    {
        if (std::getline(infile, stringbuffer, ','))
        {
            buffer[i] = stod(stringbuffer, nullptr);
        }
        else
        {
            break;
        }
    }

    if (i != 8) //We failed if we read less than 8 elements by now
    {           //We compare against 8 because i is incremented after the loop runs
        return false;
    }

    //Now we can deal with the last element of the row...
    if (std::getline(infile, stringbuffer)) //No delimiter == \n
    {
        buffer[8] = stod(stringbuffer, nullptr);
    }
    else
    {
        return false;
    }

    accIn.assign(buffer[0], buffer[1], buffer[2]);
    angvIn.assign(buffer[3], buffer[4], buffer[5]);
    angIn.assign(buffer[6], buffer[7], buffer[8]);
    return true;
}

/*
 *  readMCISinputsBin
 * 
 * Reads a set of six doubles to serve as inputs for MCIS
 * X,Y,Z acceleration, roll, pitch, yaw rates
 * 
 * Returns true on success and false on failure
 */
bool readMCISinputsBin(std::istream& infile, MCISvector& accIn, MCISvector& angIn)
{
    double buffer[] = {0,0,0,0,0,0};

    //Try to read six doubles from the stream
    infile.read((char *)&buffer, 6*sizeof(double));

    if (!infile.good())
    {
        return false;
    }

    accIn.assign(buffer[0], buffer[1], buffer[2]);
    angIn.assign(buffer[3], buffer[4], buffer[5]);
    return true;
}

/*
 *  writeMCISinputs
 * 
 * Writes a set of nine doubles from the MCIS input
 * X,Y,Z acceleration, roll, pitch, yaw rate, roll, pitch, yaw angle 
 */
void writeMCISinputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn,
                    const MCISvector& attIn)
{
    outfile << accIn.getVal(0) << ',' << accIn.getVal(1) << ',' << accIn.getVal(2) << ',';
    outfile << angIn.getVal(0) << ',' << angIn.getVal(1) << ',' << angIn.getVal(2) << ',';
    outfile << attIn.getVal(0) << ',' << attIn.getVal(1) << ',' << attIn.getVal(2);
}

/*
 *  writeMCISoutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 * 
 */
void writeMCISoutputs(std::ostream& outfile, 
                    const MCISvector& accIn, 
                    const MCISvector& angIn)
{
    writeBaseMCISoutputs(outfile, accIn, angIn);
    outfile << std::endl;
}

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
                    const MCISvector& angIn)
{
    outfile << accIn.getVal(0) << ',' << accIn.getVal(1) << ',' << accIn.getVal(2) << ',';
    outfile << angIn.getVal(0) << ',' << angIn.getVal(1) << ',' << angIn.getVal(2);
}

/*
 *  writeMCISfullOutputs
 * 
 * Writes a set of six doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 */
void writeMCISfullOutputs(std::ostream& outfile, 
                        const MCISvector& accIn, 
                        const MCISvector& angIn,
                        const MCISvector& noTCin)
{
    writeBaseMCISoutputs(outfile, accIn, angIn);
    outfile << ',' << noTCin.getVal(0) << ',' << noTCin.getVal(1) << ',' << noTCin.getVal(2);
    outfile << std::endl;
}


/*
 *  Write MDA log
 * 
 * Writes six inputs (a_x, a_y, a_z, p, q, r) and six outputs 
 * (x, y, z, phi, theta, psi) to a CSV file
 */
void write_MDA_log(std::ostream& outfile,
                    const MCISvector& acc_in,
                    const MCISvector& angv_in,
                    const MCISvector& ang_in,
                    const MCISvector& pos_out,
                    const MCISvector& ang_out)
{
    writeMCISinputs(outfile, acc_in, angv_in, ang_in);
    outfile << ",";
    writeBaseMCISoutputs(outfile, pos_out, ang_out);
    outfile << std::endl;
}




/*
 *  writeMCISfullOutputsBin
 * 
 * Binary version
 * 
 * Writes a set of nine doubles from the MCIS output
 * X,Y,Z position, roll, pitch, yaw
 * 
 * Careful with the endianness...
 */
void writeMCISfullOutputsBin(std::ostream& outfile, 
                        const MCISvector& accIn, 
                        const MCISvector& angIn,
                        const MCISvector& noTCin)
{
    double buffer[9];

    buffer[0] = accIn.getVal(0);
    buffer[1] = accIn.getVal(1);
    buffer[2] = accIn.getVal(2);

    buffer[3] = angIn.getVal(0);
    buffer[4] = angIn.getVal(1);
    buffer[5] = angIn.getVal(2);

    buffer[6] = noTCin.getVal(0);
    buffer[7] = noTCin.getVal(1);
    buffer[8] = noTCin.getVal(2);

    outfile.write((char *)buffer, 9*sizeof(double));
}