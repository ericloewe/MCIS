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
#include <cassert>
#include <unistd.h>
#include <math.h>
#include "MCIS_xplane_sock.h"












/*
 *  xplaneSocket constructor
 * 
 * It is responsible for opening the socket and setting up the correct type
 * of message to be received.
 * 
 * It also spawns the thread that will actually receive stuff
 */
xplaneSocket::xplaneSocket(uint16_t localPort, xplaneMsgType msgType)
{
    //Boilerplate socket setup
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 1)
    {
        std::runtime_error invalid_sock_fd_exception("Socket creation failed!\n");
        throw invalid_sock_fd_exception;
    }

    //Bind the socket 
    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(localPort);
    if (bind(sock_fd, (sockaddr*)&localAddr, sizeof(struct sockaddr_in)) == -1)
    {
        std::runtime_error invalid_sock_fd_exception("Failed to bind socket to INADDR_ANY!\n");
        throw invalid_sock_fd_exception;
    }

    /*
     * Spawn the recv thread
     * 
     * This thread will execute recvThreadFunc.
     * 
     * Note that non-static member functions always receive this as their first
     * argument. recvThreadFunc nominally takes no arguments, so we just need'
     * to include the normally-implicit this pointer.
     */
    recvThread = std::thread(&xplaneSocket::recvThreadFunc, this);
}

/*
 *  xplaneSocket destructor
 * 
 * Steps taken:
 * 
 * 1) Tell the recv thread function to stop
 * 2) Wait for the recv thread to join
 * 3) Close the socket
 */
xplaneSocket::~xplaneSocket()
{
    continueRecv = false;
    recvThread.join();
    close(sock_fd);
}

/*
 *  recvThreadFunc
 * 
 * This functions runs in the recv thread and listens on the socket, waiting
 * for new messages.
 * 
 */
void xplaneSocket::recvThreadFunc()
{
    struct sockaddr_in  recvAddr;
    unsigned int recvAddrSize;
    int receivedBytes;
    
    std::cout << "recvThread started..." << std::endl;

    while (continueRecv)
    {
        //The cast is to silence -Wconversion. We are NEVER going to receive
        //more than 2^32 bytes at once.
        receivedBytes = (int)recvfrom(sock_fd, (void *)&rawMsg, XP9_MSG_SIZE,
                                   0, (sockaddr*)&recvAddr, &recvAddrSize);
        
        if (receivedBytes == -1)
        {
            //Todo - make this an exception and catch upstack
            std::cerr << "Receive socket returned -1" << std::endl;
            exit(1);
        }
        
        if (messageVersion == XP9)
        {
            //Check the message length
            if (receivedBytes != XP9_MSG_SIZE)
            {
                std::cerr << "Message received has wrong length for X-Plane 9 message." << std::endl;
                std::cerr << "Should be: " << sizeof(xplane9msg) << "  Is: " << receivedBytes << std::endl;
                continue;
            }
            //std::cout << "Message received\n";
            interpretXP9msg();
        }

        else if (messageVersion == XP11)
        {
            //Ooops
            std::logic_error except("You're trying to use XP11 without coding for it, dumbass!\n");
            throw except;
        }
    }


}

/*
 *  interpretXP9msg
 * 
 * Interpret a message sent by X-Plane 9's data output as used by the SVI.
 * 
 * This function acquires the lock that prevents reading of incomplete
 * output buffers
 */
void xplaneSocket::interpretXP9msg()
{
    double xSf, ySf, zSf, p, q, r;

    xSf = *(float *) (msgPointer + xplane9msg::offset_sfX);
    ySf = *(float *) (msgPointer + xplane9msg::offset_sfY);
    zSf = *(float *) (msgPointer + xplane9msg::offset_sfZ);

    p   = *(float *) (msgPointer + xplane9msg::offset_p);
    q   = *(float *) (msgPointer + xplane9msg::offset_q);
    r   = *(float *) (msgPointer + xplane9msg::offset_r);
    
    #ifdef SYSTEM_IS_BIG_ENDIAN

        //Ooops
        std::logic_error except("Big-endian systems are not yet supported!\n");
        throw except;

    #endif //SYSTEM_IS_BIG_ENDIAN

    /*
     *  Accelerations are in g  and need to be converted to m/s^2
     */
    xSf *= gravity;
    ySf *= gravity;
    zSf *= gravity;

    /*
     *  Angular velocities are in degrees per second. We need rad/s
     */
    p *= M_PI/180;
    q *= M_PI/180;
    r *= M_PI/180;

    //std::cout << xSf << "," << ySf << "," << zSf << "," << p << "," << q << "," << r << std::endl;  

    //This lock guard locks the mutex until it is destructed, when this function returns.
    std::lock_guard<std::mutex> lock(stateMutex);

    sfBuffer.assign(xSf, ySf, zSf);
    angBuffer.assign(p, q, r);
}

/*
 *  Getter for the buffered received output
 * 
 * Of note is the fact that it locks a mutex to prevent the data from being
 * clobbered by the recv thread while it is being copied off
 */
void xplaneSocket::getData(MCISvector& spForces, MCISvector& angVelocities)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    spForces        = sfBuffer;
    angVelocities   = angBuffer;
}