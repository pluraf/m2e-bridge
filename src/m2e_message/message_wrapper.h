#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include <regex>

#include "m2e_aliases.h"
#include "message.h"



class MessageWrapper{
public:
    MessageWrapper() = default;
    MessageWrapper(Message const & msg){
        orig_ = msg;
        alt_ = msg;
        is_initialized_ = true;
        is_passed_ = true;
    }
    Message const & orig(){return orig_;}
    Message & msg(){return alt_;}

    operator bool()const{return is_initialized_;}

    bool is_passed(){return is_passed_;}
    void pass(){is_passed_ = true;}
    void reject(){is_passed_ = false;}
    void pass_if(bool cond){is_passed_ = cond;}

    void add_destination(string queuid){destinations_.insert(queuid);}
    template<class Cont>
    void add_destinations(Cont const & cont){destinations_.insert(cont.begin(), cont.end());}
    set<string> const & get_destinations(){return destinations_;}
    void clear_destinations(){destinations_.clear();}

private:
    Message orig_;
    Message alt_;
    bool is_initialized_ {false};
    bool is_passed_ {false};
    set<string> destinations_;
};


#endif  // __M2E_BRIDGE_MESSAGE_WRAPPER_H__