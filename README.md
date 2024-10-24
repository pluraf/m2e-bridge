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
cd build
cmake -DPAHO_BUILD_STATIC=TRUE -DPAHO_WITH_SSL=TRUE -DPAHO_ENABLE_TESTING=FALSE ..
cd ..
make -j8
cd ../..
cmake -Bbuild -H. -DPAHO_BUILD_SAMPLES=ON
```
##### Install dependencies

```
sudo apt install libfmt-dev
sudo apt install libzmq3-dev
```
##### Build package

```
cd build  
cmake --preset default ..  
make -j6  
```

To run built package from m2e-bridge folder:
./build/m2e-bridge <config file path>

eg:
./build/m2e-bridge configs/m2e-bridge.json

### Integrations ###

#### GCP Pub/Sub

https://cloud.google.com/pubsub/docs/publish-receive-messages-client-library#pubsub-client-libraries-cpp

To get service key: In the Google Cloud Console, go to IAM & Admin > Service Accounts
 and create a new service account or download key from an existing service account
### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines:
 Update version of m2e-bridge in src/VERSION.h file when making changes.

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact
