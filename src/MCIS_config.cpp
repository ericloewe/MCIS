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
#include "include/MCIS_config.h" 

/*
 *  Constructors
 */

//Default constructor - does nothing
MCISconfig::MCISconfig()
{}

//Initialization constructor - loads and verifies a config file
MCISconfig::MCISconfig(std::string filename)
{
    if (!load(filename))
    {
        std::runtime_error except(
            "Failed to load MDA config\n");
        throw except;
    }
}

/*
 *  Other functions
 */

bool MCISconfig::load(std::string filename)
{
    return true;
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