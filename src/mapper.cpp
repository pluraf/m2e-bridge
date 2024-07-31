#include "mapper.h"


void Mapper::apply(MessageWrapper const &msg_w) {
    if (condition_.isTrue(msg_w)) {
        Route route = router_.getRoute(msg_w);
        connector_out_.send(msg_w.msg, route);
    }
}