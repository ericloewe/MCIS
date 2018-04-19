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
 * This header defines the interface for a MOOG 6DOF2000E Motion Base, using 
 * the Ethernet interface. 
 */

#pragma once

#include <cstdint>

/* MB sample rate in Hz */
#define MB_SAMPLE_RATE 60

/* Motion Command Word definitions */
#define MCW_DISABLE         220
#define MCW_PARK            210
#define MCW_LOW_LIM_ENABLE  200
#define MCW_LOW_LIM_DISABLE 190
#define MCW_ENGAGE          180
#define MCW_START           175
#define MCW_LENGTH_MODE     172
#define MCW_DOF_MODE        170
#define MCW_RESET           160
#define MCW_INHIBIT         150
#define MCW_MDA_MODE        140
#define MCW_NEW_POSITION    130

/* Latched fault data masks */
//All faults are asserted on 1
#define FAULT_ESTOP         0x8000
#define FAULT_SNUBBER       0x4000
#define FAULT_ACT_RUNAWAY   0x2000
#define FAULT_BATTERY       0x1000
#define FAULT_LOW_IDLE_RATE 0x0800
#define FAULT_MOTOR_THERMAL 0x0400
#define FAULT_CMD_RANGE_ERR 0x0200
#define FAULT_INVALID_FRAME 0x0100

#define FAULT_WATCHDOG      0x0080
#define FAULT_LIMIT_SWITCH  0x0040
#define FAULT_DRIVE_BUS     0x0020
#define FAULT_AMPLIFIER     0x0010
#define FAULT_COMM          0x0008
#define FAULT_HOMING        0x0004
#define FAULT_ENVELOPE      0x0002
#define FAULT_TORQUE_MON    0x0001

/* Discrete I/O information */
//All conditions are asserted on 1
#define INFO_ESTOP_SENSE        0x80
#define INFO_AMP_ENABLE_CMD     0x40
#define INFO_DRIVE_BUS_SENSE    0x20
#define INFO_LIM_SHUNT_CMD      0x10
#define INFO_LIM_SWITCH_SENSE   0x08
#define INFO_AMP_FAULT_SENSE    0x04
#define INFO_THERM_FAULT_SENSE  0x02
#define INFO_BASE_AT_HOME       0x01

/* Machine state information masks */
#define MASK_STATE_FEEDBACK_TYPE    0x80
#define MASK_STATE_CMD_MODE         0x60
#define MASK_STATE_ENCODED          0x0F

/* Machine state Command Modes */
#define STATE_CMD_MODE_LENGTH   0x00
#define STATE_CMD_MODE_DOF      0x20
#define STATE_CMD_MODE_MDA      0x40
#define STATE_CMD_MODE_INVALID  0x60

/* Encoded machine states */
#define MB_STATE_POWER_UP   0x0
#define MB_STATE_IDLE       0x1
#define MB_STATE_STANDBY    0x2
#define MB_STATE_ENGAGED    0x3
#define MB_STATE_PARKING    0x7
#define MB_STATE_FAULT1     0x8
#define MB_STATE_FAULT2     0x9
#define MB_STATE_FAULT3     0xA
#define MB_STATE_DISABLED   0xB
#define MB_STATE_INHIBITED  0xC


/*
    Packet format to send DOF commands to the MB

    All elements besides MCW are actually Big-Endian single-precision floats.
    They're stored as uint32s to ensure that they're not messed with.

    The floatHostToNet function can be used to convert machine-format floats
    to the necessary format.

    SPARE is not used.
*/
struct DOFpacket
{
    public:
    uint32_t MCW;
    uint32_t roll_cmd;
    uint32_t pitch_cmd;
    uint32_t heave_cmd;
    uint32_t surge_cmd;
    uint32_t yaw_cmd;
    uint32_t lateral_cmd;
    uint32_t SPARE = 0;
};

/*
 *  Packet format used by the MB to respond to DOF commands
 * 
 * Feedback elements are actually Big-Endian single-precision floats.
 * They're stored as uint32s to ensure that they're not messed with.
 * 
 * The floatNetToHost function can be used to extract machine-format floats
 * from the raw values
 * 
 * SPARE is not used.
 */
struct DOFresponse
{
    public:
    uint32_t latched_fault_data;
    uint32_t discrete_IO_info;
    uint32_t machine_state_info;
    uint32_t roll_feedback;
    uint32_t pitch_feedback;
    uint32_t heave_feedback;
    uint32_t surge_feedback;
    uint32_t yaw_feedback;
    uint32_t lateral_feedback;
    uint32_t SPARE = 0;
};




