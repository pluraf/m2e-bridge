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


#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include <regex>

#include "m2e_aliases.h"
#include "message.h"


class MessageWrapper{
    std::shared_ptr<Message> orig_;
    std::shared_ptr<Message> alt_;
    bool is_initialized_ {false};
    bool is_passed_ {false};
    set<string> destinations_;
    json metadata_;
public:
    MessageWrapper() = default;
    MessageWrapper(std::shared_ptr<Message> msg_ptr){
        orig_ = msg_ptr;
        alt_ = std::make_shared<Message>(* msg_ptr.get());
        is_initialized_ = true;
        is_passed_ = true;
        metadata_ = json();
    }
    Message const & orig()const{return * orig_.get();}
    Message & msg(){return * alt_.get();}

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


#endif  // __M2E_BRIDGE_MESSAGE_WRAPPER_H__