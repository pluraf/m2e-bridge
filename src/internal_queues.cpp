#include "internal_queues.h"


InternalQueues * InternalQueues::instance_ = nullptr;
map<string, TSQueue<Message>> InternalQueues::queues_;