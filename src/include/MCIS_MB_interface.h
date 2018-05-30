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

#include <fstream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "MCIS_MDA.h"
#include "MCIS_xplane_sock.h"
#include "discreteMath.h"
#include "MOOG6DOF2000E.h"

enum iface_status   {ESTABLISH_COMMS, WAIT_FOR_ENGAGE, ENGAGING, 
                     WAIT_FOR_READY, RATE_LIMITED, ENGAGED, PARKING, MB_FAULT,
                     MB_RECOVERABLE_FAULT};

//These are all self-explanatory
enum iface_error    {NONE, MB_FAULT_1, MB_FAULT_2, MB_FAULT_3, 
                     MB_RESPONSE_TIMED_OUT, MB_ENGAGE_FAILED, 
                     MB_ESTOP};

class mbinterface
{
    private:
    
    bool continue_operation = true;
    
    MCISvector curr_pos_out, curr_rot_out;
    MCISvector curr_acceleration_in, curr_ang_velocity_in;
    const MCISvector init_pos_out{MB_OFFSET_x, MB_OFFSET_y, MB_OFFSET_z};
    const MCISvector init_rot_out{MB_OFFSET_roll, MB_OFFSET_pitch, MB_OFFSET_yaw};
    //The rate limits are defined per sample
    //0.34 mm/sample ~= 20 mm/s
    double pos_rate_lim = 3.4e-4;
    //0.016 degree/sample ~= 1 degree/s
    double rot_rate_lim = 0.016;

    vectorRateLimit pos_rate_limiter{pos_rate_lim, init_pos_out};
    vectorRateLimit rot_rate_limiter{rot_rate_lim, init_rot_out};

    std::mutex output_mutex;

    iface_status current_status = ESTABLISH_COMMS;
    iface_error  current_error = NONE;

    std::thread MB_recv_thread;
    std::thread MB_send_thread;

    unsigned long int send_ticks = 1;
    unsigned long int ticks_per_tock = 2;
    unsigned long int engage_timeout_period     = 1200;
    unsigned long int rate_limit_timeout_period = 1200;

    unsigned long int state_start; 
    //int DOF_mode_ticks = 60;

    //int recv_sock_fd;
    int send_sock_fd;

    bool sock_bound = false;

    //uint16_t recv_port;
    uint16_t send_port;

    //struct sockaddr_in recvAddr;
    struct sockaddr_in sendAddr;

    bool userEngage = false;
    bool userReady  = false;
    bool userPark   = false;
    bool userOverride = false;
    bool userReset = false;

    bool MB_error_asserted = false;
    uint32_t MB_state_reply = 0xFFFFFFFF;
    uint32_t MB_state_info_raw = 0xFFFFFFFF;

    //std::chrono::time_point<std::chrono::high_resolution_clock> state_start;
    //std::chrono::time_point<std::chrono::high_resolution_clock> state_current;

    //std::chrono::duration<int64_t, std::nano> delay10s = std::chrono::nanoseconds((int) 1e10);
    //std::chrono::duration<int64_t, std::nano> delay15s = std::chrono::nanoseconds((int) 1.5e10);

    //std::chrono::duration<std::chrono::high_resolution_clock> 10s_timeout = 
    //    std::chrono::seconds((int) 10);



    xplaneSocket simSocket;
    MCIS_MDA mda;

    std::fstream *MDA_logfile;

    void mb_recv_func();
    void mb_send_func();

    void mb_send_func_ESTABLISH_COMMS();
    void mb_send_func_WAIT_FOR_ENGAGE();
    void mb_send_func_ENGAGING();
    void mb_send_func_WAIT_FOR_READY();
    void mb_send_func_RATE_LIMITED();
    void mb_send_func_ENGAGED();
    void mb_send_func_PARKING();
    void mb_send_func_MB_FAULT();
    void mb_send_func_MB_RECOVERABLE_FAULT();

    void send_mb_command(int MCW, MCISvector& pos, MCISvector& rot);
    void send_mb_neutral_command(int MCW);
    void testsend_mb_command();

    void reset_user_commands();

    static void output_limiter(MCISvector& pos, MCISvector& rot);


    public:
    
    void stop();
    
    mbinterface(uint16_t mb_send_port, uint16_t mb_recv_port, uint32_t mb_IP,
                uint16_t xp_recv_port, MCISconfig mdaconfig, 
                std::fstream& MDA_log);
    //~mbinterface();

    void setEngage();
    void setReady();
    void setPark();
    void setOverride();
    void setReset();

    int get_ticks();

    unsigned int get_MB_status();
    iface_status get_iface_status();
    void get_MDA_status(MCISvector& sf_in, MCISvector& angv_in, 
                        MCISvector& MB_pos_out, MCISvector& MB_rot_out);

};
