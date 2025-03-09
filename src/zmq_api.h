/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#ifndef __M2E_BRIDGE_ZMQ_API_H__
#define __M2E_BRIDGE_ZMQ_API_H__


#include <zmq.hpp>

#include "m2e_aliases.h"


class ZMQAPI{
    string api_handler(unsigned char const * buffer, size_t len);
    string execute_request(char const * method, char const * path, unsigned char const * payload, size_t payload_len);
    string zmq_channel_get(char const * path, unsigned char const * payload, size_t payload_len);
    string zmq_channel_put(char const * path, unsigned char const * payload, size_t payload_len);
    string zmq_channel_post(char const * path, unsigned char const * payload, size_t payload_len);
    string zmq_channel_delete(char const * path, unsigned char const * payload, size_t payload_len);
public:
    string handle_message(zmq::message_t const & request);
};


#endif  // __M2E_BRIDGE_ZMQ_API_H__
