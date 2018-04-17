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

#include <thread>
#include <chrono>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "discreteMath.h"
#include "MOOG6DOF2000E.h"

enum iface_status   {ESTABLISH_COMMS, WAIT_FOR_ENGAGE, ENGAGING, 
                     WAIT_FOR_READY, RATE_LIMITED, ENGAGED, PARKING};

class mbinterface
{
    private:
    MCISvector curr_pos_out;
    MCISvector curr_rot_out;

    iface_status current_status;

    std::thread MB_recv_thread;
    std::thread MB_send_thread;

    int recv_sock_fd;
    int send_sock_fd;

    uint16_t recv_port;
    uint16_t send_port;

    struct sockaddr_in recvAddr;
    struct sockaddr_in sendAddr;

    void mb_recv_func();
    void mb_send_func();

    void mb_send_func_ESTABLISH_COMMS();
    void mb_send_func_WAIT_FOR_ENGAGE();
    void mb_send_func_ENGAGING();
    void mb_send_func_WAIT_FOR_READY();
    void mb_send_func_RATE_LIMITED();
    void mb_send_func_ENGAGED();
    void mb_send_func_PARKING();

    void send_mb_command(int MCW, MCISvector pos, MCISvector rot);


    public:
    mbinterface(uint16_t mb_send_port, uint16_t mb_recv_port, uint32_t mb_IP);
    ~mbinterface();

};
