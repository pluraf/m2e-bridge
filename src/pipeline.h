#include <string>
#include <vector>
#include <thread>


class Pipeline {
    using std::vector;
    using std::string;
public:
    static Pipeline createFromDescription(std::string);
    void start();
    void stop();
private:
    void run();
    Connector connector_in_;
    vector<FilterBase> filters_;
    vector<TransformerBase> transformers_;
    vector<Mapper> mappers_;

    bool stop_ { false };
    std::thread *th_ { nullptr };
};
