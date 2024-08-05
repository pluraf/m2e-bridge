
#include "nlohmann/json.hpp"

#include "connectors/connector.h"


class ConnectorFactory {
public:
    static Connector* create(nlohmann::json json_descr) {}
};