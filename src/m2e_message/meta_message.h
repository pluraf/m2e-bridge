/* SPDX-License-Identifier: BSD-3-Clause */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __M2E_BRIDGE_META_MESSAGE_H__
#define __M2E_BRIDGE_META_MESSAGE_H__


#include <regex>

#include "m2e_aliases.h"
#include "message.h"


class MetaMessage{
    std::shared_ptr<Message> orig_;
    std::shared_ptr<Message> alt_;
    bool is_initialized_ {false};
    bool is_passed_ {false};
    set<string> destinations_;
    json metadata_;
public:
    MetaMessage() = default;

    MetaMessage(std::shared_ptr<Message> msg_ptr){
        orig_ = msg_ptr;
        alt_ = std::make_shared<Message>();
        is_initialized_ = true;
        is_passed_ = true;
        metadata_ = json();
    }

    Message const & orig()const{return * orig_.get();}

    Message & msg(){
        if(! * alt_){
            alt_ = std::make_shared<Message>(* orig_.get());
        }
        return * alt_.get();
    }

    void set_message(Message && msg){
        alt_ = std::make_shared<Message>(msg);
    }

    std::shared_ptr<Message> msg_ptr(){return alt_;}
    explicit operator bool()const{return is_initialized_;}
    bool is_passed(){return is_passed_;}
    void pass(){is_passed_ = true;}
    void reject(){is_passed_ = false;}
    void pass_if(bool cond){is_passed_ = cond;}
    void add_destination(string queuid){destinations_.insert(queuid);}
    template<class Cont>
    void add_destinations(Cont const & cont){destinations_.insert(cont.begin(), cont.end());}
    set<string> const & get_destinations(){return destinations_;}
    void clear_destinations(){destinations_.clear();}

    void add_metadata(json const & metadata){
        metadata_.merge_patch(metadata);
    }

    json const & get_metadata(){
        return metadata_;
    }
};


#endif  // __M2E_BRIDGE_META_MESSAGE_H__