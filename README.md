# README #

This README would normally document whatever steps are necessary to get your application up and running.

### What is this repository for? ###

* Quick summary
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

### How do I get set up? ###

##### [Setup VCPKG (C++ package manager)](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash)

* Make sure you use Clang++ 18 [https://clang.llvm.org/](https://clang.llvm.org/)

Checkout submodules
```
git submodule update --init --checkout --recursive
```
Build `paho-mqtt-c`
```
cd ./external/paho.mqtt.cpp/externals/paho-mqtt-c/
mkdir build
cmake -DPAHO_BUILD_STATIC=TRUE -DPAHO_WITH_SSL=TRUE -DPAHO_ENABLE_TESTING=FALSE ..
make -j8
```


cd build  
cmake --preset default ..  
make -j6  

### Integrations ###

#### GCP Pub/Sub

https://cloud.google.com/pubsub/docs/publish-receive-messages-client-library#pubsub-client-libraries-cpp

To get service key: In the Google Cloud Console, go to IAM & Admin > Service Accounts
 and create a new service account or download key from an existing service account
### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact
