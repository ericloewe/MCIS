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


#include <vector>
#include <stdexcept>
#include "MCIS_MDA.h"










/*
 *  MCIS_MDA constructor
 * 
 * This one is very simple. It just forwards the configuration reference
 * to the member class constructors and sets the gains for the input scaling
 * again using the config reference.
 */
MCIS_MDA::MCIS_MDA(const MCISconfig& config)
    :   angleBlock{config},
        tiltBlock{config},
        posBlock{config},
        posOut{0,0,0},
        angleOut{0,0,0},
        angleNoTCout{0,0,0},
        accInput{0,0,0},
        angInput{0,0,0},
        kX{config.K_SF_x},
        kY{config.K_SF_y},
        kZ{config.K_SF_z},
        kp{config.K_p}, 
        kq{config.K_q},
        kr{config.K_r}
{}

/*
 *  MCIS_MDA::nextSample
 * 
 * Run one iteration of the MCIS MDA
 * 
 * Theory of operation / signal path:
 * 
 * 1) - NOT IMPLEMENTED YET - Linear acceleration is offset from CG
 * 2) Inputs are scaled
 * 3) Scaled angular velocities from 2) are used as input for the 
 *      Motion Base Orientation block, angleBlock, by calling
 *      angleBlock.nextSample().
 *      The output is stored in angleNoTCout
 * 4) The output of angleBlock from 3) is fed into the Tilt Coordination Block,
 *      tiltBlock, to be used along with the scaled linear accelerations from 
 *      2) in tiltBlock.nextSample().
 *      The output is stored in angleOut.
 * 5) The output of tiltBlock from 4) is used along with the scaled linear 
 *      accelerations from 2) as the input for the Motion Base Position block,
 *      posBlock, specifically posBlock.nextSample()
 *      The output is stored in posOut.
 * 
 * All outputs can be later retrieved using the getter functions.
 */
void MCIS_MDA::nextSample(const MCISvector& accelerations, const MCISvector& angularVelocities)
{
    accInput = accelerations;
    angInput = angularVelocities;
    
    // 2) Scale inputs
    accInput.applyScalarGains(kX, kY, kZ);
    angInput.applyScalarGains(kp, kq, kr);

    // 3) Calculate the Motion Base position from the angular velocity input
    angleNoTCout = angleBlock.nextSample(angInput);

    // 4) Calculate Tilt Coordination using known MB orientation and acceleration input
    angleOut = tiltBlock.nextSample(accInput, angleNoTCout);

    // 5) Calculate the Motion Base position from the acceleration input
    posOut = posBlock.nextSample(accInput, angleOut);
}

/*
 *  MCIS_MDA getters
 * 
 * These functions are used to get the results from the MCIS MDA algorithm
 * after each iteration
 */
MCISvector& MCIS_MDA::getPos()
{
    return posOut;
}
MCISvector& MCIS_MDA::getangle()
{
    return angleOut;
}
MCISvector& MCIS_MDA::getAngleNoTC()
{
    return angleNoTCout;
}










/*
 *  body2inert
 * 
 * Rotates the frame of reference of the input vector from body axes to
 * pseudo-inertial Earth-fixed axes.
 * 
 * We generate a Direction Cosines Matrix from the given Euler Angles,
 * which we then immediately invert.
 * Well, that's a bit of a lie. DCMs are orthogonal matrices, so instead
 * of inverting, we can simply transpose it. And since we can transpose it,
 * we might as well generate it as the transpose of the DCM from the start.
 * 
 * Anyway, we can then simply right-multiply the vector we wish to move
 * from body to pseudo-inertial axes with the inverse DCM and the overwritten
 * vector will now be expressed in Earth-fixed axes.
 */
void body2inert(MCISvector& vec, const MCISvector& eulerAngles)
{
    //Generate an empty matrix which we will soon fill with the DCM contents
    MCISmatrix DCM{};
    DCM.euler2DCM_ZYX_inv(eulerAngles);

    //Our DCM is ready, we can multiply the vector
    vec = DCM.rightMultiplyVector(vec);
}









/*
 *  angHPchannel constructor
 * 
 * Takes an MCISconfig reference and constructs its members
 * using members of the MCISconfig instance
 */
angHPchannel::angHPchannel(const MCISconfig& config) 
    :   rollFilt{config.filt_p_HP_disc.biquads[0]},
        pitchFilt{config.filt_q_HP_disc.biquads[0]},
        yawFilt{config.filt_r_HP_disc.biquads[0]},
        rollSat{config.lim_p, 0},
        pitchSat{config.lim_q, 0},
        yawSat{config.lim_r, 0},
        rollFiltK{config.filt_p_HP_disc.biquads[0].gain},
        pitchFiltK{config.filt_q_HP_disc.biquads[0].gain},
        yawFiltK{config.filt_r_HP_disc.biquads[0].gain},
        lastOutput{0,0,0}
{}

/*
 *  angHPchannel::nextSample
 * 
 * Run one iteration of the filter bank
 * 
 * Theory of operation/signal path:
 * 1) Input signal frame of reference is rotated to inertial frame using
 *      the previous period's output
 * 2) The vector is split  up into its three components
 * 3) Inputs are clamped down to their limits (saturation)
 * 4) Each element is used as the input for the 
 *      the respective filter (which includes the integrator)
 * 5) Filter output gets reassembled into an MCISvector and is returned
 *      and stored for the next iteration.
 */
MCISvector angHPchannel::nextSample(MCISvector& input)
{
    // 1) Rotate input's frame of reference from body to inertial
    body2inert(input, lastOutput);

    // 2) Split up the vector
    double pChannel = input.getVal(0);
    double qChannel = input.getVal(1);
    double rChannel = input.getVal(2);

    // 3) Run the inputs through the saturations
    pChannel = rollSat.nextSample(pChannel);
    qChannel = pitchSat.nextSample(qChannel);
    rChannel = yawSat.nextSample(rChannel);

    // 4) Run the saturated inputs through the filters
    pChannel = rollFiltK  * rollFilt.nextSample(pChannel);
    qChannel = pitchFiltK * pitchFilt.nextSample(qChannel);
    rChannel = yawFiltK   * yawFilt.nextSample(rChannel);

    lastOutput.assign(pChannel, qChannel, rChannel);
    return lastOutput;
}

/*
 *  posHPchannel constructor
 * 
 * Takes an MCISconfig reference and constructs its members
 * using members of the MCISconfig instance
 */
posHPchannel::posHPchannel(const MCISconfig& config)
    :   xFilt1{config.filt_SF_HP_x_disc.biquads[0]},
        yFilt1{config.filt_SF_HP_y_disc.biquads[0]},
        zFilt1{config.filt_SF_HP_z_disc.biquads[0]},
        xFilt2{config.filt_SF_HP_x_disc.biquads[1]},
        yFilt2{config.filt_SF_HP_y_disc.biquads[1]},
        zFilt2{config.filt_SF_HP_z_disc.biquads[1]},
        xFiltK{config.filt_SF_HP_x_disc.biquads[0].gain},
        yFiltK{config.filt_SF_HP_y_disc.biquads[0].gain},
        zFiltK{config.filt_SF_HP_z_disc.biquads[0].gain},
        xSat{config.lim_SF_x, 0},
        ySat{config.lim_SF_y, 0},
        zSat{config.lim_SF_z, 0}
{
    zGravSub = gravity * config.K_SF_z;
}

/*
 *  posHPchannel::nextSample
 * 
 * Run one iteration of the filter bank
 * 
 * Theory of operation/signal path:
 * 1) Input signal frame of reference is rotated to inertial frame using
 *      the MBangles input vector, corresponding to the orientation
 *      calculated before this function is called 
 *      (inclusive of Tilt Coordination).
 * 2) The vector is split  up into its three components
 * 3) Gravity is subtracted from the Z axis to bring it down to the 
 *      [-limit ; limit] range.
 * 4) Inputs are clamped down to their limits (saturation)
 * 5) Each element is used as the input for the 
 *      the respective filter (which includes the integrator)
 * 6) Filter output gets reassembled into an MCISvector
 */
MCISvector posHPchannel::nextSample(MCISvector& input, const MCISvector& MBangles)
{
    // 1) Rotate input's frame of reference from body to inertial
    body2inert(input, MBangles);

    // 2) Split up the vector
    double xChannel = input.getVal(0);
    double yChannel = input.getVal(1);
    double zChannel = input.getVal(2);

    // 3) Subtract gravity in the Z-axis
    zChannel -= zGravSub;

    // 4) Apply saturation
    xChannel = xSat.nextSample(xChannel);
    yChannel = ySat.nextSample(yChannel);
    zChannel = zSat.nextSample(zChannel);

    // 5) Run through the filters
    xChannel =  xFilt1.nextSample(xChannel);
    xChannel =  xFilt2.nextSample(xChannel);
    xChannel *= xFiltK;

    yChannel =  yFilt1.nextSample(yChannel);
    yChannel =  yFilt2.nextSample(yChannel);
    yChannel *= yFiltK;

    zChannel =  zFilt1.nextSample(zChannel);
    zChannel =  zFilt2.nextSample(zChannel);
    zChannel *= zFiltK;

    // 6) Reassemble vector and return it
    MCISvector output{xChannel, yChannel, zChannel};
    return output;
}

/*
 *  tiltCoordination constructor
 * 
 * Takes an MCISconfig reference and constructs its members
 * using members of the MCISconfig instance
 */
tiltCoordination::tiltCoordination(const MCISconfig& config)
    :   xFilt{config.filt_SF_LP_x_disc.biquads[0]},
        yFilt{config.filt_SF_LP_y_disc.biquads[0]},
        xFiltK{config.filt_SF_LP_x_disc.biquads[0].gain},
        yFiltK{config.filt_SF_LP_y_disc.biquads[0].gain},
        xSat{config.lim_TC_x, 0},
        ySat{config.lim_TC_y, 0},
        xRatelim{config.ratelim_TC_x, 0},
        yRatelim{config.ratelim_TC_y, 0},
        xGain{config.K_TC_x},
        yGain{config.K_TC_y}
{}

/*
 *  tiltCoordination::nextSample
 * 
 * Run one iteration of the filter bank
 * 
 * Theory of operation/signal path:
 * 1) Input signal frame of reference is rotated to inertial frame using
 *      the MBangles input vector, corresponding to the orientation
 *      calculated before this function is called.
 * 2) The vector is split  up into its three components and z gets dumped
 * 3) Inputs are clamped down to their limits (saturation)
 * 4) Each element is multiplied by the Tilt Coordination gain for its axis
 * 5) Each element is used as the input for the respective filter
 * 6) Each element is rate-limited   
 * 7) Filter output gets reassembled into an MCISvector - note that
 *      the output vector is constructed as [y, x, 0], because y acceleration
 *      affects the roll output, x acceleration affects the pitch output and
 *      z acceleration has no tilt coordination
 * 8) This vector is summed with the MBangles input and returned.
 */
MCISvector tiltCoordination::nextSample(MCISvector& input, const MCISvector& MBangles)
{
    // 1) Rotate input's frame of reference from body to inertial
    body2inert(input, MBangles);

    // 2) Split up the vector
    double xChannel = input.getVal(0);
    double yChannel = input.getVal(1);

    // 3) Apply saturation
    xChannel = xSat.nextSample(xChannel);
    yChannel = ySat.nextSample(yChannel);

    // 4) Apply TC gain
    xChannel *= xGain;
    yChannel *= yGain;

    // 5) Run through the filters
    xChannel = xFiltK * xFilt.nextSample(xChannel);
    yChannel = yFiltK * yFilt.nextSample(yChannel);

    // 6) Apply rate limiting
    xChannel = xRatelim.nextSample(xChannel);
    yChannel = yRatelim.nextSample(yChannel);

    // 7) Reassemble the vector
    //Note that it's [y, x, 0]
    MCISvector output{yChannel, xChannel, 0};

    // 8) Sum the tilt coordination part to the existing orientation
    output += MBangles;
    return output;
}