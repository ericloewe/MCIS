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

#include <vector>
#include "discreteMath.h"
#include "MCIS_config.h"

//#define gravity 9.81

/*
 * Forward declarations
 */

//General Motion Drive Algorithm class
class MCIS_MDA;
// Offset linear accelerations calculator class
class CGoffset;
//MB orientation high-pass filtering class
class angHPchannel;
//Tilt coordination channel class
class tiltCoordination;
//MB position high-pass filtering class
class posHPchannel;

/*
//2nd order filter bank parameters class
//OBSOLETE
class params2ndOrderFilt
{
    public:

    std::vector<double> xNum, xDen;
    std::vector<double> yNum, yDen;
    std::vector<double> zNum, zDen;

    double gain;
};*/

//Rotate frame of reference from body to inertial axes
void body2inert(MCISvector& vec, const MCISvector& eulerAngles);



/*
 *  Angular High-Pass channel class
 * 
 * This class implements a self-contained Angular High-Pass channel
 */
class angHPchannel
{
    private:
    discreteFilt2ndOrder rollFilt, pitchFilt, yawFilt;
    saturation rollSat, pitchSat, yawSat;
    double rollFiltK, pitchFiltK, yawFiltK; //Gains applied after the biquad sections

    MCISvector lastOutput;

    public:
    //Constructor
    angHPchannel(const MCISconfig& config);


    MCISvector nextSample(const MCISvector& input);

    //Not implemented, reserved for future use
    void setFilterParameters(const MCISconfig& config);
};

/*
 *  Specific Force High-Pass channel class
 * 
 * This class implements a self-contained Specific Force High-Pass channel
 */
class posHPchannel
{
    private:
    discreteFilt2ndOrder xFilt1, yFilt1, zFilt1; //We need two biquad sections
    discreteFilt2ndOrder xFilt2, yFilt2, zFilt2; //per filter for Specific Force
    double xFiltK, yFiltK, zFiltK;  //Gains applied after the biquad sections
    saturation xSat, ySat, zSat;

    double zGravSub;    //Value to subtract from the z-axis, corresponds to
                        //g*K_SF_z and is set in constructor

    public:
    //Constructor
    posHPchannel(const MCISconfig& config);


    MCISvector nextSample(const MCISvector& input, const MCISvector& MBangles);

    //Not implemented, reserved for future use
    void setFilterParameters(const MCISconfig& config);
};

/*
 *  Tilt coordination channel class
 * 
 * This class implements a self-contained Tilt Coordination channel
 */
class tiltCoordination
{
    private:
    discreteFilt2ndOrder xFilt, yFilt;
    double xFiltK, yFiltK; //Gains applied after the biquad sections
    saturation xSat, ySat;
    rateLimit xRatelim, yRatelim;
    double xGain, yGain;

    public:
    //Constructor
    tiltCoordination(const MCISconfig& config);


    MCISvector nextSample(const MCISvector& input, const MCISvector& MBangles);

    //Not implemented, reserved for future use
    void setFilterParameters(const MCISconfig& config);
};

/*
 *  MCIS MDA class
 * 
 * This class implements a single object that provides the full MCIS algorithm.
 * 
 * MDA stands for Motion Drive Algorithm
 */
class MCIS_MDA
{
    private:

    angHPchannel        angleBlock;
    tiltCoordination    tiltBlock;
    posHPchannel        posBlock;

    MCISvector posOut, angleOut, angleNoTCout;
    MCISvector accInput, angInput;

    double kX, kY, kZ, kp, kq, kr;

    public:

    MCIS_MDA(const MCISconfig& config);

    void nextSample(const MCISvector& accelerations, const MCISvector& angularVelocities);
    MCISvector& getPos();
    MCISvector& getangle();
    MCISvector& getAngleNoTC();
};
