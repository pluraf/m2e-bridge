#include <vector>
#include <utility>


class Mapper {
public:
    void map(MessageWrapper &msg_w);
    void start();
    void stop();
private:
    Condition condition_;
    Router router_;
    Connector connector_out_;
};