#include "mapper.h"


Mapper::map(MessageWrapper const &msg_w) {
    if (condition_.isTrue(msg_w)) {
        Route route = router.getRoute(msg_w);
        connector_.send(msg_w.msg, route);
    }
}