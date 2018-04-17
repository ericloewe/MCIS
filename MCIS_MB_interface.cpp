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

#include "MCIS_MB_interface.h"

mbinterface::mbinterface(uint16_t mb_send_port, uint16_t mb_recv_port, uint32_t mb_IP)
{
    recv_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    send_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (recv_sock_fd < 1 || send_sock_fd < 1)
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
    }

    //Bind send socket
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_addr.s_addr = htonl(mb_IP);
    sendAddr.sin_port = htons(mb_send_port);
    if (bind(send_sock_fd, (sockaddr*)&sendAddr, sizeof(struct sockaddr_in)) == -1)
    {
        std::runtime_error invalid_sock_fd_exception("Failed to bind socket to MB IP!\n");
        throw invalid_sock_fd_exception;
    }

    //We start in the first state
    current_status = ESTABLISH_COMMS;

    //Spawn other threads
    MB_recv_thread = std::thread(&mbinterface::mb_recv_func, this);
    MB_send_thread = std::thread(&mbinterface::mb_send_func, this);

}