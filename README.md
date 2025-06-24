# Eclipse Paho MQTT C++ Client Library

This repository contains the source code for the [Eclipse Paho](http://eclipse.org/paho) MQTT C++ client library for memory-managed operating systems such as Linux, MacOS, and Windows.

This code builds a library which enables Modern C++ applications (C++17 and beyond) to connect to an [MQTT](http://mqtt.org) broker, publish messages, subscribe to topics, and receive messages from the broker.

The library has the following features:

- Support for MQTT v3.1, v3.1.1, and v5.
- Network Transports:
    - Standard TCP
    - UNIX-domain sockets
    - Secure sockets with SSL/TLS
    - WebSockets
        - Secure and insecure
        - Proxy support
- Message persistence
    - User configurable
    - Built-in File persistence
    - User-defined key/value persistence easy to implement
- Automatic Reconnect
- Offline Buffering
- High Availability
- Blocking and non-blocking APIs
- Modern C++ interface (C++17)

This code requires the [Paho C library](https://github.com/eclipse/paho.mqtt.c) by Ian Craggs, et al., specifically version 1.3.14 or possibly later.

## Latest News

To keep up with the latest announcements for this project, or to ask questions:

**Email:** [Eclipse Paho Mailing List](https://accounts.eclipse.org/mailing-list/paho-dev)

### What's New in v1.5.x

The latest updates for v1.5 moved the codebase to C++17 and added support for UNIX-domain sockets. They also fixed a number of build issues, now targeting the latest Paho C release, v1.3.14.

The primary changes in the v1.5 versions are:

- Updated the code base to C++17
- Support for the Paho C v1.3.14 release.
    - Support for UNIX-domain sockets
- Reorganize and reformat the sources and added a .clang-format capability.
- Create universal client instances that can connect using v3 or v5. (i.e. no more instances that are only v3 capable)
- Bump the CMake to v3.13
- Fix a large number of CMake build issues
- Updated the GitHub CI
- (v1.5.3) Fixes for building this library and the Paho C library with the latest C & C++ compilers, like Clang 20 and GCC 15 which created some breaking hcanges for legacy code.
- (v1.5.4) Fixes for `topic_matcher` and `topic_filter` to match against parent with the multi-field wildcard.

For the full list of updates in each release, see the [CHANGELOG](https://github.com/eclipse-paho/paho.mqtt.cpp/blob/master/CHANGELOG.md).

## Contributing

Contributions to this project are gladly welcomed and appreciated. Before submitting a Pull Request, please keep three things in mind:

 - This is an official Eclipse project, so it is required that all contributors sign an [Eclipse Contributor Agreement (ECA)](https://www.eclipse.org/legal/ECA.php)
 - Please submit all Pull Requests against the _develop_ branch (not master).
 - Please sign all commits.

 For full details, see [CONTRIBUTING.md](https://github.com/eclipse/paho.mqtt.cpp/blob/master/CONTRIBUTING.md).

## Building from source

As of v1.5, the Paho C++ library uses C++17 features, thus requiring a fully compliant C++17 compiler. Some of the more common compilers that can be used, depending on the target platform, are:

* GCC v8 or later
* _clang_ v5 or later
* Visual Studio 2017 15.8 (MSVC 19.15) or later

_CMake_  is a cross-platform build system suitable for Unix and non-Unix platforms such as Microsoft Windows. It is the only supported build system. The current supported minimum version is:

* cmake v3.13

The Paho C++ library requires the Paho C library, v1.3.14 or greater to be built and installed. That can be done before building this library, or it can be done here using the CMake `PAHO_WITH_MQTT_C` build option to build both libraries at the same time. This also guarantees that a proper version of the C library is used, and that it is build with compatible options.

### Build Options

CMake allows for options to direct the build. The following are specific to Paho C++:

Variable | Default Value | Description
------------ | ------------- | -------------
PAHO_BUILD_SHARED | TRUE (*nix), FALSE (Win32) | Whether to build the shared library
PAHO_BUILD_STATIC | FALSE (*nix), TRUE (Win32) | Whether to build the static library
PAHO_WITH_SSL | TRUE (*nix), FALSE (Win32) | Whether to build SSL/TLS support into the library
PAHO_BUILD_DOCUMENTATION | FALSE | Create the HTML API documentation (requires _Doxygen_)
PAHO_BUILD_EXAMPLES | FALSE | Whether to build the example programs
PAHO_BUILD_TESTS | FALSE | Build the unit tests. (Requires _Catch2_)
PAHO_BUILD_DEB_PACKAGE | FALSE | Flag that configures cpack to build a Debian/Ubuntu package
PAHO_WITH_MQTT_C | FALSE | Whether to build the bundled Paho C library

Enabling `PAHO_WITH_MQTT_C` builds and links in the Paho C library using compatible build options. If this is enabled, it passes the `PAHO_WITH_SSL` option to the C library, and also sets the options `PAHO_HIGH_PERFORMANCE` and `PAHO_WITH_UNIX_SOCKETS` for the C lib. These can be disabled in the cache before building if desired.

In addition, the C++ build might commonly use `CMAKE_PREFIX_PATH` to help the build system find the location of the Paho C library if it was built separately.

### Build the Paho C++ and Paho C libraries together

The quickest and easiest way to build Paho C++ is to build it together with Paho C in a single step using the included Git submodule.
This requires the CMake option `PAHO_WITH_MQTT_C` set.

```
$ git clone https://github.com/eclipse/paho.mqtt.cpp
$ cd paho.mqtt.cpp
$ git co v1.5.4

$ git submodule init
$ git submodule update

$ cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_EXAMPLES=ON
$ sudo cmake --build build/ --target install
```

This assumes the build tools and dependencies, such as OpenSSL, have already been installed. For more details and platform-specific requirements, see below.

### Unix-style Systems (Linux, macOS, etc)

On *nix systems CMake creates Makefiles.

The build process currently supports a number of Unix and Linux flavors. The build process requires the following tools:

* CMake v3.13 or newer
* A fully-compatible C++17 compiler. Common options are:
    * GCC v8 or later
    * _clang_ v5 or later

On Debian based systems this would mean that the following packages have to be installed:

```
$ sudo apt-get install build-essential gcc make cmake
```

If you will be using secure sockets (and you probably should if you're sending messages across a public netwok):

```
$ sudo apt-get install libssl-dev
```

Building the documentation requires doxygen and optionally graphviz to be installed:

```
$ sudo apt-get install doxygen graphviz
```

Unit tests are built using _Catch2_.

_Catch2_ can be found here: [Catch2](https://github.com/catchorg/Catch2).  You must download and install _Catch2_ to build and run the unit tests locally. Currently _Catch2_ versions v2.x and v3.x are supported.

#### Building the Paho C library

The Paho C library can be built automatically when building this library by enabling the CMake build option, `PAHO_WITH_MQTT_C`. That will build and install the Paho C library from a Git submodule, using a known-good version, and the proper build configuration for the C++ library. But iIf you want to manually specify the build configuration of the Paho C library or use a different version, then it must be built and installed before building the C++ library. Note, this version of the C++ library requires Paho C v1.3.14 or greater.

To download and build the Paho C library:

    $ git clone https://github.com/eclipse/paho.mqtt.c.git
    $ cd paho.mqtt.c
    $ git checkout v1.3.14

    $ cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON
    $ sudo cmake --build build/ --target install

This builds the C library with SSL/TLS enabled. If that is not desired, omit the `-DPAHO_WITH_SSL=ON`.

It also uses the "high performance" option of the C library to disable more extensive internal memory checks. Remove the _PAHO_HIGH_PERFORMANCE_ option (i.e. turn it off) to debug memory issues, but for most production systems, leave it on for better performance.

The above will install the library to the default location on the host, which for Linux is normally `/usr/local`. To install the library to a non-standard location, use the `CMAKE_INSTALL_PREFIX` to specify a location. For example, to install into a directory under the user's home directory, perhaps for local testing, do this:

    $ cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF \
        -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON \
        -DCMAKE_INSTALL_PREFIX=$HOME/install

#### Building the Paho C++ library

If the Paho C library is not already installed, the recommended version can be built along with the C++ library in a single step using the CMake option `PAHO_WITH_MQTT_C` set on.

    $ git clone https://github.com/eclipse/paho.mqtt.cpp
    $ cd paho.mqtt.cpp
    $ git co v1.5.4
    $ git submodule init
    $ git submodule update

    $ cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_EXAMPLES=ON
    $ sudo cmake --build build/ --target install

If a recent version of the Paho C library is available on the build host, and it's installed to a default location, it does not need to be built again. Omit the `PAHO_WITH_MQTT_C` option:

    $ cmake -Bbuild -H. -DPAHO_BUILD_SAMPLES=ON

If the Paho C library is installed to a _non-default_ location, or you want to build against a different version, use the `CMAKE_PREFIX_PATH` to specify its install location. Perhaps something like this:

    $ cmake -Bbuild -H. -DPAHO_BUILD_SAMPLES=ON -DCMAKE_PREFIX_PATH=$HOME/install

#### Building a Debian/Ubuntu package

A Debian/Ubuntu install `.deb` file can be created as follows:

```
$ cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_ENABLE_TESTING=OFF -DPAHO_BUILD_DEB_PACKAGE=ON
$ cmake --build build
$ (cd build && cpack)
```

### Windows

On Windows, CMake creates Visual Studio project files for use with MSVC. Currently, other compilers like _clang_ or _MinGW_ are not directly supported.

#### Using Paho C++ as a Windows DLL

The project can be built as a static library or shared DLL on Windows. If using it as a DLL in your application, you should define the macro `PAHO_MQTTPP_IMPORTS` before including any Paho C++ include files. Preferably, make it a global definition in the application's build file, like in CMake:

    target_compile_definitions(myapp PUBLIC PAHO_MQTTPP_IMPORTS)

It's better not to mix DLLs and static libraries, but if you do link the Paho C++ DLL against the Paho C static library, you may need to manually resolve some system dependencies, like adding the WinSock library as a dependency to your application:

    target_link_libraries(myapp ws2_32)

#### Building the Library on Windows

The build process currently supports a number Windows versions. The build process requires the following tools:
  * CMake GUI v3.13 or newer
  * Visual Studio 2019 or newer

The libraries can be completely built at an MSBuild Command Prompt. Download the Paho C and C++ library sources, then open a command window and first compile the Paho C library:

    > cd paho.mqtt.c
    > cmake -Bbuild -H. -DCMAKE_INSTALL_PREFIX=C:\mqtt\paho-c
    > cmake --build build/ --target install

Then build the C++ library:

    > cd ..\paho.mqtt.cpp
    > cmake -Bbuild -H. -DCMAKE_INSTALL_PREFIX=C:\mqtt\paho-cpp -DPAHO_BUILD_SAMPLES=ON -DPAHO_WITH_SSL=OFF -DCMAKE_PREFIX_PATH=C:\mqtt\paho-c
    > cmake --build build/ --target install

This builds and installs both libraries to a non-standard location under `C:\mqtt`. Modify this location as desired or use the default location, but either way, the C++ library will most likely need to be told where the C library was built using `CMAKE_PREFIX_PATH`.

It seems quite odd, but even on a 64-bit system using a 64-bit compiler, MSVC seems to default to a 32-bit build target.

The 64-bit target can be selected using the CMake generator switch, *-G*, at configuration time. The full version must be provided.

    > cmake -G "Visual Studio 16 2019" -Ax64 -Bbuild -H. -DCMAKE_INSTALL_PREFIX=C:\mqtt\paho-c
    > ...

*Note that it is very important that you use the same generator (target) to build BOTH libraries, otherwise you will get lots of linker errors when you try to build the C++ library.*

## Supported Network Protocols

The library supports connecting to an MQTT server/broker using TCP, SSL/TLS, and websockets both (secure and insecure). On *nix targets, UNIX-domain sockets are also supported. The underlying transport is chosen by the URI supplied to indicate the remote host. It can be specified as:

    "mqtt://<host>:<port>"   - TCP, unsecure
     "tcp://<host>:<port>"    (same)

    "mqtts://<host>:<port>"  - SSL/TLS
     "ssl://<host>:<port>"     (same)

    "ws://<host>:<port>"    - Unsecure websockets
    "wss://<host>:<port>"   - Secure websockets

	"unix://<path>"          - A UNIX-domain socket on the local machine.
	                           (*nix systems, only)

The "mqtt://" and "tcp://" schemas are identical. They indicate an insecure connection over TCP. The "mqtt://" variation is new for the library, but becoming more common across different MQTT libraries.

Similarly, the "mqtts://" and "ssl://" schemas are identical. They specify a secure connection over SSL/TLS sockets.

Note that to use any of the secure connect options, "mqtts://, "ssl://", or "wss://" you must compile the library with the `PAHO_WITH_SSL=ON` CMake option to include OpenSSL. In addition, you _must_ specify `ssl_options` when you connect to the broker - i.e. you must add an instance of `ssl_options` to the `connect_options` when calling `connect()`.

The use of Unix-domain sockets is only available on *nix-style systems like Linux and macOS. It is not available on Windows. It requires the Paho C library built with the CMake option of PAHO_WITH_UNIX_SOCKETS=ON. This is done by default when building the C library automatically with the Git submodule.

## _Catch2_ Unit Tests

Unit tests use _Catch2_ for the test framework. Versions 2.x and 3.x are supported.

_Catch2_ can be found here: [Catch2](https://github.com/catchorg/Catch2)

## Basics of Thread Safety

Some things to keep in mind when using the library in a multi-threaded application:

- The clients are thread-safe. You can publish/subscribe/etc from multiple threads simultaneously. There are internal mutexes to protect multi-threaded access.
- You should not make a blocking call from within a callback from the library, i.e. anything registered with `set_callback()`, `set_message_callback()`, etc. Callbacks are invoked from the one internal thread that is processing incoming packets from the network. If you make a blocking call that expects an ACK, you will deadlock.
- You can only register one `on_message()` callback per client to receive incoming messages for all of your registered subscriptions. That callback runs in the context of the library thread. If you want to process incoming messages from a different (or multiple) threads:
    - Use a consumer queue, or create one or more instances of a thread-safe queue to move the messages around.
    - The [thread_queue](https://github.com/eclipse/paho.mqtt.cpp/blob/master/include/mqtt/thread_queue.h) class in the library is a thread-safe queue that you can use for this.
    - To route incoming messages by topic:
        - Use an instance of the (topic_matcher)[https://github.com/eclipse/paho.mqtt.cpp/blob/master/include/mqtt/topic_matcher.h] collection to create a collection of queues or callback functions to receive messages that match a set of topic filters.
        - For MQTT v5 consider using Subscription Identifiers to map incoming messages to callbacks or queues.
- The various data and options structs (like connect_options) are simple data structs. They are not thread protected.

## Examples

Sample applications can be found in the source repository at [examples/](https://github.com/eclipse/paho.mqtt.cpp/tree/master/examples).

These can all be build along with the library by specifying the CMake flag: `-DPAHO_BUILD_EXAMPLES=ON` when configuring the build.

This is a partial example of what a typical application might look like:

```cpp
int main(int argc, char* argv[])
{
    sample_mem_persistence persist;
    mqtt::client cli(ADDRESS, CLIENT_ID, &persist);

    callback cb;
    cli.set_callback(cb);

    auto connOpts = mqtt::connect_options_builder()
        .keep_alive_interval(20);
        .clean_session()
        .finalize();

    try {
        cli.connect(connOpts);

        // First use a message pointer.

        mqtt::message_ptr pubmsg = mqtt::make_message(PAYLOAD1);
        pubmsg->set_qos(QOS);
        cli.publish(TOPIC, pubmsg);

        // Now try with itemized publish.

        cli.publish(TOPIC, PAYLOAD2, strlen(PAYLOAD2)+1, 0, false);

        // Disconnect

        cli.disconnect();
    }
    catch (const mqtt::persistence_exception& exc) {
        cerr << "Persistence Error: " << exc.what() << " ["
            << exc.get_reason_code() << "]" << endl;
        return 1;
    }
    catch (const mqtt::exception& exc) {
        cerr << "Error: " << exc.what() << " ["
            << exc.get_reason_code() << "]" << endl;
        return 1;
    }

    return 0;
}
```

This code requires:

The Paho C library by Ian Craggs, et al.
https://github.com/eclipse/paho.mqtt.c