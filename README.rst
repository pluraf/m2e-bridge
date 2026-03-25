**M2E-Bridge** is an IoT processing unit designed for real-time data collection,
transformation, and routing. It ensures efficient information flow between
connected systems and supports data filtering, protocol conversion, and data generation.

For detailed documentation, see the full `docs <https://docs.iotplan.io/m2e-bridge>`_.


Building
========

You will need `vcpkg <https://vcpkg.io>`_ and `CMake <https://cmake.org>`_ to build the project.

The build has been tested on Debian 13 "trixie". As a prerequisite, you need to install the following packages::

    libfmt-dev libzmq3-dev autoconf autoconf-archive automake libtool libxi-dev libxtst-dev libxrandr-dev

* After cloning the repository, initialize the git submodules::

    git submodule update --init --checkout

* Run the script to setup the build environment::

    ./setup.sh

  M2E-Bridge is highly modular, so you can enable or disable individual
  components with fine-grained control. By default, everything is turned on,
  which results in a larger binary. If binary size matters to you, disable any
  features you don't need.

* To build the project, run::

    cmake --build build/ --parallel 8


Testing
=======

`Catch2 <https://github.com/catchorg/Catch2>`_ is used as the test framework. To build tests,
enable them during setup using CMake ``ENABLE_TESTS`` option::

  ENABLE_TESTS=ON

Run the full test suite with::

  ./build/tests/m2e-bridge-tests


Running
=======

To launch M2E-Bridge from the development tree::

  ./build/m2e-bridge configs/m2e-bridge.json
