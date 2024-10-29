#ifndef __M2E_BRIDGE_AWS_SDK_MANAGER_H__
#define __M2E_BRIDGE_AWS_SDK_MANAGER_H__

#include <aws/core/Aws.h>

class AwsSdkManager {
public:
    static AwsSdkManager& Instance(){
        static AwsSdkManager instance;
        return instance;
    }

private:
    Aws::SDKOptions options_;
    AwsSdkManager(){
        Aws::InitAPI(options_);
    }
    ~AwsSdkManager(){
        Aws::ShutdownAPI(options_);
    }

    AwsSdkManager(const AwsSdkManager&) = delete;
    AwsSdkManager& operator=(const AwsSdkManager&) = delete;
};

#endif;