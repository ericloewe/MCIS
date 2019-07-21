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
#include "include/MOOG6DOF2000E.h"
#include "include/MCIS_MB_interface.h"
#include "include/MCIS_util.h"
#include "include/MCIS_fileio.h"

mbinterface::mbinterface(uint16_t mb_send_port, uint16_t mb_recv_port, 
                         uint32_t mb_IP, uint16_t xp_recv_port, 
                         MCISconfig mdaconfig, std::fstream& MDA_log,
                         bool subtract_gravity) :
                         simSocket{xp_recv_port, XP9},
                         mda{mdaconfig, subtract_gravity},
                         subgrav{subtract_gravity}
{
    send_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    //Send socket addressing
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_addr.s_addr = htonl(mb_IP);
    sendAddr.sin_port = htons(mb_send_port);

    MDA_logfile = &MDA_log;

    //We start in the first state
    current_status = ESTABLISH_COMMS;

    //Spawn other threads
    MB_recv_thread = std::thread(&mbinterface::mb_recv_func, this);
    MB_send_thread = std::thread(&mbinterface::mb_send_func, this);
}

void mbinterface::stop()
{
    simSocket.stop();
    continue_operation = false;

    MB_send_thread.join();

    int ret = shutdown(send_sock_fd, SHUT_RDWR);
    if (ret != 0 && errno != ENOTCONN)
    {
        std::runtime_error except(
            "MB Socket shutdown did not return 0. You're deep in undefined behavior now.\n");
        throw except;
    }

    MB_recv_thread.join();
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

void mbinterface::setReset()
{
    userReset = true;
}

int mbinterface::get_ticks()
{
    return send_ticks;
}

unsigned int mbinterface::get_MB_status()
{
    return MB_state_reply;
}

iface_status mbinterface::get_iface_status()
{
    return current_status;
}

void mbinterface::get_MDA_status(MCISvector& sf_in, MCISvector& angv_in, MCISvector& ang_in,
                        MCISvector& MB_pos_out, MCISvector& MB_rot_out)
{
    //Mutex is locked here, to insure consistent reads
    std::lock_guard<std::mutex> lock(output_mutex);
    sf_in   = curr_acceleration_in;
    angv_in = curr_ang_velocity_in;
    ang_in  = curr_attitude_in;
    MB_pos_out  = curr_pos_out;
    MB_rot_out  = curr_rot_out; 
    //Mutex is unlocked here, as the lock guard is destructed at the end of its scope
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
    //std::cout << "Send thread spawned!" << std::endl;

    auto nextTick = std::chrono::high_resolution_clock::now();
    //std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));
    auto sampleTime = std::chrono::nanoseconds( (int)(1e9 / 120));
    //auto sampleTime = std::chrono::microseconds((unsigned long)(10e9));

    send_mb_neutral_command(MCW_DOF_MODE);
    sock_bound = true;

    while (continue_operation)
    {
        nextTick += sampleTime;

        if (send_ticks % ticks_per_tock == 0)
        {
            
            if (!(current_status == ESTABLISH_COMMS)    && 
                !(current_status == WAIT_FOR_ENGAGE)    &&
                !(current_status == MB_FAULT)           &&
                !(current_status == MB_RECOVERABLE_FAULT))
            {
                if (userPark)
                {
                    current_status = PARKING;
                }
                
            }

            //Handle MB faults
            if (MB_state_reply == MB_STATE_FAULT1 || 
                MB_state_reply == MB_STATE_FAULT2 ||
                MB_state_reply == MB_STATE_FAULT3 )
            {
                current_status = MB_FAULT;
            }

            //Communicate with MB
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
                case MB_FAULT:
                    mb_send_func_MB_FAULT();
                    break;
                case MB_RECOVERABLE_FAULT:
                    mb_send_func_MB_RECOVERABLE_FAULT();
                    break;
            }
            //Delete any unused user input
            reset_user_commands();
        }
        // This block exists only to allow the lock guard object to be 
        // destructed before the loop iteration ends.
        {
            //Lock the mutex
            std::lock_guard<std::mutex> lock(output_mutex);
            simSocket.getData(curr_acceleration_in, curr_ang_velocity_in, curr_attitude_in);
            mda.nextSample(curr_acceleration_in, curr_ang_velocity_in, curr_attitude_in);
            curr_pos_out = mda.getPos();
            curr_rot_out = mda.getangle();
            //Mutex is unlocked here, as the lock guard is destructed due to end of scope
            write_MDA_log(*MDA_logfile, curr_acceleration_in, curr_ang_velocity_in,
                            curr_attitude_in, curr_pos_out, curr_rot_out);
        }

        /*Clamp outputs down and offset them if needed (z)*/
        output_limiter(curr_pos_out, curr_rot_out);

        send_ticks++;
        std::this_thread::sleep_until(nextTick);
    }
}


/*
 *  recv_func
 * 
 * Loop around, receiving the MB's replies
 * 
 * TODO - consider adding a timeout
 */
void mbinterface::mb_recv_func()
{
    DOFresponse mb_response;
    static_assert(sizeof(DOFresponse) == 40, 
                "DOF response structure does not match the correct size (probably due to padding)");
    
    while (!sock_bound)
    {
        sleep(1);
    }

    while(continue_operation)
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

/*
 *  ---=== send function definitions ===---
 */

/*
 *  ESTABLISH_COMMS state function
 * 
 * We just wait until the MB state is updated, then switch to the next state
 */
void mbinterface::mb_send_func_ESTABLISH_COMMS()
{
    send_mb_neutral_command(MCW_DOF_MODE);

    if (MB_state_info_raw != 0xFFFFFFFF)
    {
        current_status = WAIT_FOR_ENGAGE;
    }
}

/*
 *  WAIT_FOR_ENGAGE state function
 * 
 * We have to wait for the user to command an ENGAGE
 */
void mbinterface::mb_send_func_WAIT_FOR_ENGAGE()
{
    send_mb_neutral_command(MCW_NEW_POSITION);

    if (userEngage)
    {
        current_status = ENGAGING;
        state_start = send_ticks;
    }
}

/*
 *  ENGAGING state function
 * 
 * We have to wait for the MB to engage.
 * We timeout after a few seconds and treat it as a failure
 */
void mbinterface::mb_send_func_ENGAGING()
{
    send_mb_neutral_command(MCW_START);

    if (MB_state_reply == MB_STATE_ENGAGED)
    {
        current_status = WAIT_FOR_READY;
        return;
    }

    //Check if we timed out
    //state_current = std::chrono::high_resolution_clock::now();
    //auto period = state_current - state_start;
    auto elapsed = send_ticks - state_start;
    if (elapsed > engage_timeout_period)
    {
        //We have timed out
        current_status  = MB_FAULT;
        current_error   = MB_ENGAGE_FAILED; 
    }
}

/*
 *  WAIT_FOR_READY state function
 * 
 * We hold the MB in the neutral position until told to start moving
 * by the user.
 */
void mbinterface::mb_send_func_WAIT_FOR_READY()
{
    send_mb_neutral_command(MCW_NEW_POSITION);

    if (userReady)
    {
        current_status = RATE_LIMITED;
        //Set the timer for the next state
        //state_start = std::chrono::high_resolution_clock::now();
        state_start = send_ticks;
        //Reset the rate limiters
        pos_rate_limiter.overrideOutput(init_pos_out);
        rot_rate_limiter.overrideOutput(init_rot_out);
    }
}

/*
 *  RATE_LIMITED state function
 * 
 * We have to rate limit the output for a few seconds to allow for non-neutral
 * starting positions, which basically happen *all* the time.
 */
void mbinterface::mb_send_func_RATE_LIMITED()
{
    //First apply the rate limits
    pos_rate_limiter.nextSample(curr_pos_out);
    rot_rate_limiter.nextSample(curr_rot_out);

    //Now we can send as normal
    send_mb_command(MCW_NEW_POSITION, curr_pos_out, curr_rot_out);

    //Verify if we're ready to move on
    //state_current = std::chrono::high_resolution_clock::now();
    //auto period = state_current - state_start;
    auto elapsed = send_ticks - state_start;
    if (elapsed > rate_limit_timeout_period)
    {
        //We're ready
        current_status = ENGAGED;
    }
}

/*
 *  ENGAGED state function
 * 
 * We just send commands. Nothing more. Parking is handled up the stack
 */
void mbinterface::mb_send_func_ENGAGED()
{
    send_mb_command(MCW_NEW_POSITION, curr_pos_out, curr_rot_out);
}

/*
 *  PARKING state function
 * 
 * We transition back to WAIT_FOR_ENGAGE when the MB states switches to idle
 */
void mbinterface::mb_send_func_PARKING()
{
    send_mb_neutral_command(MCW_PARK);

    if (MB_state_reply == MB_STATE_IDLE)
    {
        current_status = WAIT_FOR_ENGAGE;
    }
}

/*
 *  MB_FAULT state function
 * 
 * There is some fault with the MB. Currently, we send a park command,
 * but this behavior is debatable.
 * 
 * This state requires the application to be restarted.
 */
void mbinterface::mb_send_func_MB_FAULT()
{
    send_mb_neutral_command(MCW_PARK);
}

/*
 *  MB_RECOVERABLE_FAULT state function
 * 
 * This state represents recoverable faults. MB Fault 2s are sometimes 
 * recoverable. Currently, we send a park command,
 * but this behavior is debatable.
 * 
 * If faced with an unknown FAULT 2, consult the Moog 6DOF2000E User's manual
 * document number CDS7238 before proceeding with opperation.
 */
void mbinterface::mb_send_func_MB_RECOVERABLE_FAULT()
{
    send_mb_neutral_command(MCW_PARK);

    // We only cancel the error condition if the MB has done so already
    if (MB_state_reply == MB_STATE_IDLE)
    {
        current_status = WAIT_FOR_ENGAGE;
    }

    // The RESET command is sent out of sync, in addition to the regular 60Hz
    // commands. This shouldn't be a problem, since the MB has to be parked to
    // act upon a RESET command.
    // Nevertheless, keep this in mind in case something very weird happens.
    if (userReset)
    {
        send_mb_neutral_command(MCW_RESET);
        userReset = false;
    }
}

/*
 *  ***** send function definitions end here *****
 */

/*
 *  reset user commands to false to respect POLA
 * 
 * Users get astonished if their accidental input early on
 * ends up getting interpreted later in the program when the state that
 * uses that input is reached.
 */
void mbinterface::reset_user_commands()
{
    userEngage  = false;
    userReady   = false;
    userPark    = false;
    userReset   = false;
}

/*
 *  output_limiter
 * 
 * Limit outputs to the range accepted by the MB
 * 
 * Note that a Z-offset is required for Rev E11 software, but not
 * for Rev E16. The offset calculation can be optimized out if Rev E16 is used
 * (though -O3 will probably do it automagically, since the limits are 
 * preprocessor #defines).
 */
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