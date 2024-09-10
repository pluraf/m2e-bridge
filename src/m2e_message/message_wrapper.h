#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include <regex>

#include "m2e_aliases.h"
#include "message.h"


class MessageWrapper {
public:
    MessageWrapper(const Message msg): msg(msg) { }
    json const & get_payload(){
        if(! is_payload_decoded_){
            decoded_payload_ = json::parse(msg.get_text());
            is_payload_decoded_ = true;
        }
        return decoded_payload_;
    }

    std::string const & get_topic_level(int level){
        using namespace std;
        if(! is_topic_levels_){
            regex r("/");
            sregex_token_iterator it(msg.get_topic().begin(), msg.get_topic().end(), r, -1);
            sregex_token_iterator end;
            topic_levels_ = vector<std::string>(it, end);
            is_topic_levels_ = true;
        }
        return level < topic_levels_.size() ? topic_levels_[level] : empty_string_;
    }

    Message const msg;

private:
    json decoded_payload_;
    bool is_payload_decoded_ {false};
    std::vector<std::string> topic_levels_;
    bool is_topic_levels_ {false};
    std::string const empty_string_ {""};
};


#endif