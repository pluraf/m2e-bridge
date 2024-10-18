#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include <regex>

#include "m2e_aliases.h"
#include "message.h"



class MessageWrapper{
public:
    MessageWrapper() = default;
    MessageWrapper(Message const & msg):orig_(msg),alt_(msg),is_initialized_(true){}
    Message const & orig(){return orig_;}
    Message & msg(){return alt_;}

    operator bool()const{return is_initialized_;}

    bool is_passed(){return is_passed_;}
    void accept(){is_passed_ = true;}
    void reject(){is_passed_ = false;}

    void add_destination(string queuid){destinations_.insert(queuid);}
    set<string> const & get_destinations(){return destinations_;}
    void clear_destinations(){destinations_.clear();}

private:
    Message orig_;
    Message alt_;
    bool is_initialized_;
    bool is_passed_;
    set<string> destinations_;
};


#endif  // __M2E_BRIDGE_MESSAGE_WRAPPER_H__