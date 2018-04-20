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

#include <cassert>
#include <chrono>
#include <iostream>
#include <type_traits>
#include <unistd.h>
#include "MOOG6DOF2000E.h"
#include "MCIS_MB_interface.h"
#include "MCIS_util.h"

mbinterface::mbinterface(uint16_t mb_send_port, uint16_t mb_recv_port, 
                         uint32_t mb_IP, uint16_t xp_recv_port, 
                         MCISconfig mdaconfig) :
                         simSocket{xp_recv_port, XP9},
                         mda{mdaconfig}
{
    //recv_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    send_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    /*if (recv_sock_fd < 1 || send_sock_fd < 1)
    {
        std::runtime_error sock_open_failed_exception("Could not open MB sockets!\n");
        throw sock_open_failed_exception;
    }
    //Bind recv socket
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    recvAddr.sin_port = htons(mb_recv_port);
    if (bind(recv_sock_fd, (sockaddr*)&recvAddr, sizeof(struct sockaddr_in)) == -1)
    {
        std::runtime_error invalid_sock_fd_exception("Failed to bind socket to INADDR_ANY!\n");
        throw invalid_sock_fd_exception;
    }*/


    //Send socket addressing
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_addr.s_addr = htonl(mb_IP);
    //sendAddr.sin_addr.s_addr = INADDR_ANY;
    sendAddr.sin_port = htons(mb_send_port);
    /*if (bind(send_sock_fd, (sockaddr*)&sendAddr, sizeof(struct sockaddr_in)) == -1)
    {
        std::runtime_error invalid_sock_fd_exception("Failed to bind socket to MB IP!\n");
        throw invalid_sock_fd_exception;
    }*/

    //We start in the first state
    current_status = ESTABLISH_COMMS;

    //Spawn other threads
    MB_recv_thread = std::thread(&mbinterface::mb_recv_func, this);
    MB_send_thread = std::thread(&mbinterface::mb_send_func, this);


}

void mbinterface::setEngage()
{
    if (current_status == WAIT_FOR_ENGAGE)
    {
        userEngage = true;
    }
}

void mbinterface::setPark()
{
    userPark = true;
}

void mbinterface::setReady()
{
    if (current_status == WAIT_FOR_READY)
    {
        userReady = true;
    }
}

void mbinterface::setOverride()
{
    userOverride = true;
}

void mbinterface::testsend_mb_command()
{
    DOFpacket testPacket;

    testPacket.MCW = htonl(MCW_NEW_POSITION);
    testPacket.roll_cmd     = 0;
    testPacket.pitch_cmd    = 0;
    testPacket.yaw_cmd      = 0;
    testPacket.surge_cmd    = 0;
    testPacket.lateral_cmd  = 0;
    testPacket.heave_cmd    = 0;

    static_assert(sizeof(testPacket) == 32, 
                "DOF packet structure does not match the correct size (probably due to padding)");

    long int bytes = sendto(send_sock_fd, (void*)&testPacket, sizeof(testPacket), 0, 
                        (sockaddr *)&sendAddr, sizeof(sendAddr));
    if (bytes != sizeof(testPacket))
    {
        std::cout << "Error sending! Sent " << bytes << std::endl;
    }

}


void mbinterface::send_mb_command(int MCW, MCISvector& pos, MCISvector& rot)
{
    DOFpacket packet;

    packet.MCW = htonl(MCW);

    /*Clamp outputs down and offset them if needed (z)*/
    //output_limiter(pos, rot);


    packet.surge_cmd    = floatHostToNet((float)pos.getVal(0));
    packet.lateral_cmd  = floatHostToNet((float)pos.getVal(1));
    packet.heave_cmd    = floatHostToNet((float)pos.getVal(2));

    packet.roll_cmd     = floatHostToNet((float)rot.getVal(0));
    packet.pitch_cmd    = floatHostToNet((float)rot.getVal(1));
    packet.yaw_cmd      = floatHostToNet((float)rot.getVal(2));

    static_assert(sizeof(packet) == 32, 
                "DOF packet structure does not match the correct size (probably due to padding)");

    long int bytes = sendto(send_sock_fd, (void*)&packet, sizeof(packet), 0, 
                        (sockaddr *)&sendAddr, sizeof(sendAddr));
    if (bytes != sizeof(packet))
    {
        std::cout << "Error sending! Sent " << bytes << std::endl;
    }
}

void mbinterface::send_mb_neutral_command(int MCW)
{
    DOFpacket packet;

    packet.MCW = htonl(MCW);

    packet.surge_cmd    = floatHostToNet((float)init_pos_out.getVal(0));
    packet.lateral_cmd  = floatHostToNet((float)init_pos_out.getVal(1));
    packet.heave_cmd    = floatHostToNet((float)init_pos_out.getVal(2));

    packet.roll_cmd     = floatHostToNet((float)init_rot_out.getVal(0));
    packet.pitch_cmd    = floatHostToNet((float)init_rot_out.getVal(1));
    packet.yaw_cmd      = floatHostToNet((float)init_rot_out.getVal(2));

    static_assert(sizeof(packet) == 32, 
                "DOF packet structure does not match the correct size (probably due to padding)");

    long int bytes = sendto(send_sock_fd, (void*)&packet, sizeof(packet), 0, 
                        (sockaddr *)&sendAddr, sizeof(sendAddr));
    if (bytes != sizeof(packet))
    {
        std::cout << "Error sending! Sent " << bytes << std::endl;
    }
}


void mbinterface::mb_send_func()
{
    std::cout << "Send thread spawned!" << std::endl;

    auto nextTick = std::chrono::high_resolution_clock::now();
    //std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));
    auto sampleTime = std::chrono::nanoseconds( (int)(1e9 / 120));
    //auto sampleTime = std::chrono::microseconds((unsigned long)(10e9));

    send_mb_neutral_command(MCW_DOF_MODE);
    sock_bound = true;

    while (true)
    {
        nextTick += sampleTime;

        if (send_ticks % ticks_per_tock == 0)
        {
            //Communicate with MB
            if (!(current_status == ESTABLISH_COMMS || current_status == WAIT_FOR_ENGAGE))
            {
                if (userPark)
                {
                    current_status = PARKING;
                }
                
            }

            switch (current_status)
            {
                case ESTABLISH_COMMS:
                    mb_send_func_ESTABLISH_COMMS();
                    if (userOverride)
                    {
                        current_status = WAIT_FOR_ENGAGE;
                        userOverride = false;
                    }
                    break;
                case WAIT_FOR_ENGAGE:
                    mb_send_func_WAIT_FOR_ENGAGE();
                    if (userOverride)
                    {
                        current_status = ENGAGING;
                        userOverride = false;
                    }
                    break;
                case ENGAGING:
                    mb_send_func_ENGAGING();
                    if (userOverride)
                    {
                        current_status = WAIT_FOR_READY;
                        userOverride = false;
                    }
                    break;
                case WAIT_FOR_READY:
                    mb_send_func_WAIT_FOR_READY();
                    if (userOverride)
                    {
                        current_status = ENGAGED;
                        userOverride = false;
                    }
                    break;
                case RATE_LIMITED:
                    mb_send_func_RATE_LIMITED();
                    break;
                case ENGAGED:
                    mb_send_func_ENGAGED();
                    if (userOverride)
                    {
                        current_status = PARKING;
                        userOverride = false;
                    }
                    break;
                case PARKING:
                    mb_send_func_PARKING();
                    break;
            }
        }

        simSocket.getData(curr_acceleration_in, curr_ang_velocity_in);
        mda.nextSample(curr_acceleration_in, curr_ang_velocity_in);
        curr_pos_out = mda.getPos();
        curr_rot_out = mda.getangle();

        /*Clamp outputs down and offset them if needed (z)*/
        output_limiter(curr_pos_out, curr_rot_out);

        if (send_ticks % 64 == 0)
        {
            std::cout << "\033[2J";
            std::cout << "1 - Engage     4 - Ready     7 - Override    0 - Park" << std::endl;
            
            std::cout << "MDA inputs: " << std::endl;
            curr_acceleration_in.print(std::cout);
            std::cout << std::endl;
            curr_ang_velocity_in.print(std::cout);
            std::cout << std::endl;
            
            std::cout << "MDA outputs: " << std::endl;
            curr_pos_out.print(std::cout);
            std::cout << std::endl;
            curr_rot_out.print(std::cout);
            std::cout << std::endl;
        }
        

        send_ticks++;
        std::this_thread::sleep_until(nextTick);
    }
}



void mbinterface::mb_recv_func()
{
    DOFresponse mb_response;
    static_assert(sizeof(DOFresponse) == 40, 
                "DOF response structure does not match the correct size (probably due to padding)");
    
    while (!sock_bound)
    {
        sleep(1);
    }

    while(true)
    {
        //recv(recv_sock_fd, (void *)&mb_response, sizeof(mb_response), 0);
        recv(send_sock_fd, (void *)&mb_response, sizeof(mb_response), 0);
        if (mb_response.latched_fault_data)
        {
            MB_error_asserted = true;
        }
        MB_state_info_raw  = mb_response.machine_state_info;
        MB_state_reply =  ntohl(mb_response.machine_state_info) & MASK_STATE_ENCODED;
        //std::cout << "Received reply from MB" << std::endl;

    }
}


void mbinterface::mb_send_func_ESTABLISH_COMMS()
{
    send_mb_neutral_command(MCW_DOF_MODE);

    if (MB_state_info_raw != 0xFFFFFFFF)
    {
        std::cout << "Received reply from MB. Engage when ready." << std::endl;
        //std::cout << "Faults" << 
        current_status = WAIT_FOR_ENGAGE;
    }
}


void mbinterface::mb_send_func_WAIT_FOR_ENGAGE()
{
    send_mb_neutral_command(MCW_NEW_POSITION);

    if (userEngage)
    {
        std::cout << "Engaging." << std::endl;
        current_status = ENGAGING;
        userEngage = false;
    }

}

void mbinterface::mb_send_func_ENGAGING()
{
    send_mb_neutral_command(MCW_START);

    if (MB_state_reply == MB_STATE_ENGAGED)
    {
        std::cout << "MB ready." << std::endl;
        current_status = WAIT_FOR_READY;
    }
}


void mbinterface::mb_send_func_WAIT_FOR_READY()
{
    send_mb_neutral_command(MCW_NEW_POSITION);
    /*if (send_ticks % 256 == 0)
    {
        std::cout << "Output suppressed:" << std::endl;
        std::cout << "Pos: ";
        curr_pos_out.print(std::cout);
        std::cout << std::endl;
        std::cout << "Rot: ";
        curr_rot_out.print(std::cout);
        std::cout << std::endl;

    }*/

    if (userReady)
    {
        std::cout << "User ready, motion enabled" << std::endl;
        current_status = ENGAGED;
        userReady = false;
    }
}


void mbinterface::mb_send_func_RATE_LIMITED()
{
    //TODO - ADD THIS ONE
}

void mbinterface::mb_send_func_ENGAGED()
{
    send_mb_command(MCW_NEW_POSITION, curr_pos_out, curr_rot_out);
}


void mbinterface::mb_send_func_PARKING()
{
    send_mb_neutral_command(MCW_PARK);
    userPark = false;
}

void mbinterface::output_limiter(MCISvector& pos, MCISvector& rot)
{
    /* Offset values that need to be offset */
    //This means only the z-position
    pos.setVal(2, pos.getVal(2) + MB_OFFSET_z);
    //Other values do not need offsetting.
    //Newer versions of the 6DOF2000E software do not require offsets at all

    /* Clamp values down to acceptable ranges */
    //x-position
    if (pos.getVal(0) < MB_LIM_LOW_x)
    {
        pos.setVal(0, MB_LIM_LOW_x);
    }
    else if (pos.getVal(0) > MB_LIM_HIGH_x)
    {
        pos.setVal(0, MB_LIM_HIGH_x);
    }

    //y-position
    if (pos.getVal(1) < MB_LIM_LOW_y)
    {
        pos.setVal(1, MB_LIM_LOW_y);
    }
    else if (pos.getVal(1) > MB_LIM_HIGH_y)
    {
        pos.setVal(1, MB_LIM_HIGH_y);
    }

    //z-position
    if (pos.getVal(2) < MB_LIM_LOW_z)
    {
        pos.setVal(2, MB_LIM_LOW_z);
    }
    else if (pos.getVal(2) > MB_LIM_HIGH_z)
    {
        pos.setVal(2, MB_LIM_HIGH_z);
    }

    //Roll
    if (rot.getVal(0) < MB_LIM_LOW_roll)
    {
        rot.setVal(0, MB_LIM_LOW_roll);
    }
    else if (rot.getVal(0) > MB_LIM_HIGH_roll)
    {
        rot.setVal(0, MB_LIM_HIGH_roll);
    }

    //Pitch
    if (rot.getVal(1) < MB_LIM_LOW_pitch)
    {
        rot.setVal(1, MB_LIM_LOW_pitch);
    }
    else if (rot.getVal(1) > MB_LIM_HIGH_pitch)
    {
        rot.setVal(1, MB_LIM_HIGH_pitch);
    }

    //Yaw
    if (rot.getVal(2) < MB_LIM_LOW_yaw)
    {
        rot.setVal(2, MB_LIM_LOW_yaw);
    }
    else if (rot.getVal(2) > MB_LIM_HIGH_yaw)
    {
        rot.setVal(2, MB_LIM_HIGH_yaw);
    }

}