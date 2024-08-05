
#include "m2e_message/message.h"


class InternalQueue {
public:
    void store(Message const &msg) {}
    unsigned int claim() {}
    Message get(unsigned int claim_token) {}
};