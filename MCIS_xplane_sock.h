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


#define XP9_MSG_SIZE 185




/*
 *  Legacy X-Plane 9 message as used by the SVI
 * 
 * It is a complete disaster. The format is not really specified anywhere
 * and the person who reverse-engineered this didn'r bother to actually
 * document it. So, this is reverse-engineered from the Simulink block that 
 * was reverse-engineered through trial, error and who knows what else.
 * 
 * Total size of this monstrosity is 185 bytes, most of which are useless.
 * 
 * Endianness is unspecified. The Simulink block doesn't do anything and 
 * Simulink itself is little-endian only. So, we're assuming little-endian.
 * 
 * This is done purely for compatibility without having to mess with X-Plane.
 * The reason for that decision should be evident from the format in use...
 */
class xplane9msg
{
    public:
    //A pseudo-header for this disaster. It says "DATA"
    char header[4];
    //Nobody seems to know what this next byte does
    char unknown;

    //First packet starts here
    //Packet ID
    uint32_t id0;
    float mach;
    float pad1;
    float vvi; //???
    float pad2;
    //Linear accelerations
    //These are ordered as Z,X,Y (WTF!?) and expressed in g
    float accZ, accX, accY;
    float pad3;

    //Second packet starts here
    uint32_t id1;
    //Angular accelerations as q, p, r dot (WTF!?)
    float q_dot, p_dot, r_dot;
    float pad4[5];

    //Third packet starts here
    uint32_t id2;
    //Angular velocities as q, p, r (WTF!?)
    float q, p, r;
    float pad5[5];

    //Fourth packet
    //It only contains stuff we don't care about
    uint32_t id3;
    //Your guess is as good as mine. I have no clue what mavar is.
    float pitch, roll, hdg_true, hdg_mag, hdg_compass, pad6, pad7, mavar;

    //Fifth packet
    //It only contains stuff we don't care about, these seem to be aero angles
    uint32_t id4;
    float alpha, beta, h_path, v_path, pad8[3], slip;

    //Message ends here, 185 bytes
    //The following are definitions to grab the useful data:

    static const unsigned int offset_sfX = 29;
    static const unsigned int offset_sfY = 33;
    static const unsigned int offset_sfZ = 25;

    static const unsigned int offset_p = 85;
    static const unsigned int offset_q = 81;
    static const unsigned int offset_r = 89;
};



/*
 *  This enum is used to determine which type of message should be read
 */
enum xplaneMsgType {XP9, XP11};


/*
 *  Yup. A union. Ugh.
 * 
 * We make a union out of all the message types to have a single buffer large
 * enough to hold any of them on recv.
 */
union xplaneMsgFormats
{
    xplane9msg xp9;
};


class xplaneSocket
{
    protected:

    //File descriptor for the socket
    int sock_fd;
    struct sockaddr_in recvAddr;

    bool continueRecv = true;
    bool dataReceived = false;

    //Message type stuff
    xplaneMsgType messageVersion;
    unsigned char rawMsg[XP9_MSG_SIZE];
    unsigned char *msgPointer = (unsigned char *)&rawMsg;

    //This mutex controls access to the output buffer
    std::mutex stateMutex;
    //C++11 thread object for the recv thread (fancy fd sort of thing)
    std::thread recvThread;
    //Output buffers
    MCISvector sfBuffer{0, 0, 9.81}, angBuffer{0, 0, 0};

    //The function that loops around, receiving.
    void recvThreadFunc();
    //void init(int localPort, xplaneMsgType msgType);

    void interpretXP9msg();
    void interpretXP11msg();

    public:

    xplaneSocket(uint16_t localPort, xplaneMsgType msgType);
    ~xplaneSocket();

    
    void getData(MCISvector& spForces, MCISvector& angVelocities);



};

class xp9Socket : public xplaneSocket
{
    private:
    xplane9msg rawMsg;


};