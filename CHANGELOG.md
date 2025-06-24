# Change Log
#
# Eclipse Paho MQTT C++ Library

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

- Fixed `topic_matcher` and `topic_filter` to properly match parent with multi-level ('#') wildcard.
- Slight optimization of `topic_filter` to do simple string comparison if the filter does not contain wildcards.
- [#556](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/556) fix potential deadlock in `thread_queue` on capacity increase.
- [#559](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/559) prevent undefined behaviour on empty topic matching

## [Version 1.5.3](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.5.2..v1.5.3) (2025-05-15)

- Fix the bundled Paho C build foc C23 compilers by forcing C99 compliance in CMake build
- [#544](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/544) and [#550](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/550) Use std::vector<unsigned char> for the ALPN protocol list in wire format
- [#547](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/547) Fixed up some of the v5 examples for proper connect options
- [#549](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/549) Update TEST_EXTERNAL_SERVER urls


## [Version 1.5.2](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.5.1..v1.5.2) (2025-03-11)

- Fixed the Version number and string.
- Synchronous `Client` constructors updated to use `persistence_type` and (just) `create_options`
    - Restored compatibility with `async_client`
- [#505](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/505): Example of retrieving MQTT v5 properties in message received callback
- [#537](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/537) Fixed the Windows DLL build by exporting message::EMPTY_STR and message::EMPTY_BIN
- [#540](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/537) Missing default argument in `async_client` changed constructor breaks code compatibility


## [Version 1.5.1](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.5.0..v1.5.1) - (2025-02-09)

- Minor fixes to README and docs
- [#532](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/532) Fix CMake install target lib path
- [#534](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/534) Fixed seg fault with clang in get_topic() when publishing a message
- [#535](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/535) Fixed last few files that were not properly licenced for EPL v2.0


## [Version 1.5.0](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.4.1..v1.5.0) - (2025-01-07)

- Code base updated to to C++17
    - Now a C++17 compiler is required to compile the library
- CMake minimum required version raised to v3.13
    - Need a fairly recent CMake for C++17 support (>= v3.12)
    - [#504](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/504) CMake v3.13 allows INSTALL(TARGETS) to work outside the current directory.
- Clients always created for v5 persistence format, making it universal for any connection.
    - If the application specifies a version it is kept as a hint for default connections.
    - The version for the connection should be specified in the connect options.
- The `create_options` now have all the parameters to create a client.
    - Can specify Server URL, Client ID, and persistence in the create options.
    - New client constructor that takes just the options object
    - The client caches a const `create_options` struct with all the creation parameters
    - Client creation internally simplified without breaking the public API
- Expanded the message constmer to be a full client "event" consumer.
    - The events are for *connected, connection_lost, disconnected, message arrived,* and application *shutdown.*
    - The application can get client state change notifications without resorting to callbacks.
- There's a new `persistence_type` (std::variant) that can hold any of the persistence specifiers (none, file directory, or user interface).
- Most of the class static constants are now `constexpr`.
- Removed the fake `ReasonCode::MQTTPP_V3_CODE`. Now all reason codes in a v3 connection are SUCCESS.
- The `mqtt::exception` checks if the 'rc' return code actually contains a reason code error, amd if so, sets it as the reason code.
- `property` can now report the `typeid` of its contained value.
- The `properties` list implements a const iterator
- Added a `to_string()` and `operator<<()` for reason codes.
- `thread_queue` is now closable.
- Added documentation for UNIX domain sockets coming in with Paho C v1.3.14
- Removed the manual implementation of `make_unique<>()`
- Added `create_options` assignment operators.
- Fixed some corner cases for topic_filter::matches()
- Cleaned up and fixed a number of example apps.
    - Most apps now except a server URI from the command line
    - 'data_publish' example uses C++17 std::filesystem for creating a file-based encrypted persistence for messages.
- Updated local CI (buildtst.sh) for current compilers and unit tests.
- Reorganized the source repository
- Completely reformat the sources and added a .clang-format file (a project master and a slightly-different one for headers).
- Added GitHub CI Action, removing legacy Travis and Appveyor files
- [#410](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/410) Added 'shutdown_event' to the event consumer and reworked consumer to prevent propagating exceptions on shutdown.
- [#451](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/451) Added low keep alive to async_publish_time to test connected/connection_lost callbacks
- [#503](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/503) Fixed issue that generated docs were empty.
- [#518](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/518) Add function for checking async consumer event queue size
- [#519](https://github.com/eclipse-paho/paho.mqtt.cpp/pull/519) Fix potential deadlock in set_callback
- [#524](https://github.com/eclipse-paho/paho.mqtt.cpp/issues/524) Fixed copy and move operations for 'subscribe_options'. Added unit tests.


## [Version 1.4.1](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.4.0..v1.4.1) - (2024-07-09)

- [#458](https://github.com/eclipse/paho.mqtt.cpp/issues/458) Set 'disconnected' handler for the consumer queue.


## [Version 1.4.0](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.3.2..v1.4.0) - (2024-06-16)

- Ability to build the Paho C library automatically (now working properly)
    - CMake 'PAHO_WITH_MQTT_C' option properly compiles the existing Paho C v1.3.13
    - Moved 'src/externals/' to top-level
- Reorganized the source tree:
    - Moved header files to top-level 'include/' directory.
    - Moved 'src/sampless/' to top-level and renamed 'examples/'
- Fixed and optimized 'topic_matcher' trie collection
- Added some missing Eclipse/Paho legal documents to the repo.
- Ran a spell-checker over the code and doc files.

- [#498](https://github.com/eclipse/paho.mqtt.cpp/issues/498) Overloaded property constructor to also take a uint32_t
- [#491](https://github.com/eclipse/paho.mqtt.cpp/pull/491) add topic_matcher.h to install
- [#485](https://github.com/eclipse/paho.mqtt.cpp/pull/485) export dependencies
- [#484](https://github.com/eclipse/paho.mqtt.cpp/pull/484) add token::get_message
- [#480](https://github.com/eclipse/paho.mqtt.cpp/issues/480) Fixed Paho C version in 'install_paho_mqtt_c.sh' script.
- [#473](https://github.com/eclipse/paho.mqtt.cpp/issues/473) Getter for the client's cached connect options.
- [#466](https://github.com/eclipse/paho.mqtt.cpp/pull/466) Iterable string collection
- [#416](https://github.com/eclipse/paho.mqtt.cpp/issues/416) Removed FindPahoMqttC.cmake. Using Paho C package directly.


## [Version 1.3.2](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.3.1..v1.3.2) - (2023-12-05)

- [#463](https://github.com/eclipse/paho.mqtt.cpp/issues/463) Fixed generator expression for older CMake
- [#378](https://github.com/eclipse/paho.mqtt.cpp/issues/378) Bad LWT message in async_publish.cpp sample.


## [Version 1.3.1](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.3.0..v1.3.1) - (2023-11-23)

- [#462](https://github.com/eclipse/paho.mqtt.cpp/pull/462) Fix version string for version v1.3.x


## [Version 1.3.0](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.2.0..v1.3.0) - (2023-11-22)

- Fixed building and using library as DLL on Windows with MSVC
- Updated License to Eclipse Public License v2.0
- Updated create and connect options to better deal with MQTT protocol version
- Defaulting connect version to v5 if specified in create options.
- Added a `topic_filter` class to match a single filter to specific topics.
- Added a `topic_matcher` class to create a collection of items in a trie structure that can contain items tied to topic filters. (Useful for queues or callbacks per-subscription topic).
- Minor tweaks to prepare for C++20
- Support for Catch2 v3.x for unit tests (v2.x also still supported).
- Changed the sample apps to use the newer "mqtt://" schemas.
- Connect option initializers for v5 and WebSockets.

Fixed Issues and Pull Requests:

- [#343](https://github.com/eclipse/paho.mqtt.cpp/issues/343) async_client::try_consume_message_until taking single parameter fails to compile
- [#445](https://github.com/eclipse/paho.mqtt.cpp/pull/445) Update properties when moving/copying connect options.
- [#325](https://github.com/eclipse/paho.mqtt.cpp/issues/325) Cache connect options in client to keep memory valid for callbacks like SSL on_error()
- [#361](https://github.com/eclipse/paho.mqtt.cpp/issues/361) Added missing LICENSE file to conform to GitHub conventions.
- [#304](https://github.com/eclipse/paho.mqtt.cpp/issues/304) Missing create_options::DFLT_C_STRUCT symbol when linking with MSVC.
- [#429](https://github.com/eclipse/paho.mqtt.cpp/issues/429) Remove declaration of connect_options::to_string() with missing implementation.
- [#411](https://github.com/eclipse/paho.mqtt.cpp/issues/411) Missing virtual keyword for some client methods
- [#444](https://github.com/eclipse/paho.mqtt.cpp/issues/444) Unit tests to check that connect options builder sets properties.
- [#313](https://github.com/eclipse/paho.mqtt.cpp/issues/313) Get unit tests building on Windows. Needed to get rid of make_unique<> for Windows
- [#397](https://github.com/eclipse/paho.mqtt.cpp/issues/397) Doc about clean session in connect_options.h is wrong
- [#442](https://github.com/eclipse/paho.mqtt.cpp/issues/442) g++ complains with multiple definition of static constexpr for mixed C++11/17 builds
- [#445](https://github.com/eclipse/paho.mqtt.cpp/pull/445)Fix copy/move constructor for connect/disconnect opts with properties
- [#425](https://github.com/eclipse/paho.mqtt.cpp/pull/425) Silence warning for unused variable rsp in class `unsubscribe_response`
- [#440](https://github.com/eclipse/paho.mqtt.cpp/pull/440) Fix typos across the project
- [#428](https://github.com/eclipse/paho.mqtt.cpp/issues/428) Fixed type in create_options.h
- [#407](https://github.com/eclipse/paho.mqtt.cpp/pull/407) Fix nodiscard warnings in sync client
- [#385](https://github.com/eclipse/paho.mqtt.cpp/issues/385) Thread queue deadlock with multiple consumers
- [#374](https://github.com/eclipse/paho.mqtt.cpp/pull/374) Add Paho C as a submodeule
- [#350](https://github.com/eclipse/paho.mqtt.cpp/pull/350) avoid adding Paho MQTT C library twice
- [#253](https://github.com/eclipse/paho.mqtt.cpp/issues/253) implicit capture of 'this' via '[=]' is deprecated in C++20
- [#337](https://github.com/eclipse/paho.mqtt.cpp/issues/337) copy/move of caPath_ in ssl_options
- [#330](https://github.com/eclipse/paho.mqtt.cpp/pull/330) added /build/ folder to .gitignore
- [#317](https://github.com/eclipse/paho.mqtt.cpp/issues/317) String constructor using just len instead of end iterator.
- [#323](https://github.com/eclipse/paho.mqtt.cpp/issues/323) Added Session Expiry Interval to v5 chat sample to test.


## [Version 1.2.0](https://github.com/eclipse/paho.mqtt.cpp/compare/v1.1..v1.2.0) - (2020-12-27)

This release bring in some missing MQTT v5 features, brings in support for websocket headers and proxies, ALPN protocol lists, adds the builder pattern for options, and fixes a number of bugs in both the C++ library and the underlying C lib.

Requires Paho C v1.3.8

- Missing MQTT v5 features:
    - Ability to add properties to Subscribe and Unsubscribe packets (i.e. subscription identifiers)
    - "Disconnected" callback gives reason code and properties for server disconnect
- New `create_options` that can be used to construct a client with new features:
    - Send while disconnected before the 1st successful connection
    - Output buffer can delete oldest messages when full
    - Can choose to clear the persistence store on startup
    - Select whether to persist QoS 0 messages
- Started classes to create options using the Builder Pattern, with the `create_options_builder`, `connect_options_builder`, `message_ptr_builder`, etc.
- User-defined websocket HTTP headers.
- HTTP/S proxy support
- Added ALPN protocol support to SSL/TLS options
- SSL/TLS error and PSK callback support
- Update connection callback support (change credentials when using auto-reconnect)
- Updates to the sample apps:
    - Overall cleanup with better consistency
    - Example of using websockets and a proxy
    - User-based file persistence with simple encoding/encryption
    - Sharing a client between multiple threads
- Converted the unit tests to use Catch2
- All library exceptions are now properly derived from the `mqtt::exception` base class.
- [#231] Added `on_disconnected` callback to handle receipt of disconnect packet from server.
- [#211, #223, #235] Removed use of Log() function from the Paho C library.
- [#227] Fixed race condition in thread-safe queue
- [#224] & [#255] Subscribing to MQTT v3 broker with array of one topic causes segfault.
- [#282] Ability to build Debian/Ubuntu package
- [#300] Calling `reconnect()` was hanging forever, even when successful. In addition several of the synchronous `client` calls were hanging forever on failure. They now properly throw a `timeout_error` exception.
- Several memory issues and bug fixes from updated Paho C library support.


## Version 1.1 (2019-10-12)

This release was primarily to add MQTT v5 support and server responses.

- MQTT v5 support:
    - **Properties**
        - New `property` class acts something like a std::variant to hold a property of any supported type.
        - New `properties` class is a collection type to hold all the properties for a single transmitted packet.
        - Properties can be added to outbound messages and obtained from received messages.
        - Properties can also be obtained from server responses to requests such as from a _connect_ call. These are available in the `token` objects when they complete.
    - The client object tracks the desired MQTT version that the app requested and/or is currently connected at. Internally this is now required by the `response_options` the need to distinguish between pre-v5 and post-v5 callback functions.
    - MQTT v5 reason codes for requests are available via `token` objects when they complete. They are also available in `exception` objects that are thrown by tokens.
    - Support for subscribe options, like no local subscriptions, etc.
    - Sample applications were added showing how to do basic Remote Procedure Calls (RPC's) with MQTT v5 using the *RESPONSE_TOPIC* and *CORRELATION_DATA* properties. These are *rpc_math_cli* and *rpc_math_srvr* in the _src/samples_ directory.
    - A sample "chat" application was added, showing how to use subscribe options, such as "no local".
- More descriptive error messages (PR #154), integrated into the `mqtt::exception` class. MQTT v5 reason codes are also included in the exceptions when an error occurs.
- Applications can (finally) get server responses from the various ACK packets. These are available through the tokens after they complete, as `connect_response`, `subscribe_response`, and `unsubscribe_response`.
- The `topic` objects can be used to subscribe.
- Applications can register individual callback functions instead of using a `callback` interface object. This allows easy use of lambda functions for callbacks.
- The connect options can take a LWT as a plain message, via `connect_options::set_will_message()`
- New unit tests have started using _Catch2_.
- Tested with Paho C v1.3.1


## Version 1.0.1 (2018-12-12)

This is a bug-fix released aimed mainly at issues with the build system and working towards more "modern" usage of CMake. In addition:

- Support for Paho C v1.2.1
- Fixed a number of build issues, particularly on Windows
- Windows shared libraries (DLL's) now supported
- Several minor bug fixes


##  Version 1.0.0 (2017-07-23)

The initial Paho C++ Client library for memory-managed platforms (Linux, Windows, etc).

- Requires Paho C Client Library v1.2.
- MQTT 3.1 & 3.1.1
- SSL/TLS
- Asynchronous & Synchronous interfaces
- Persistence and off-line buffering
- Automatic reconnect
- High availability.
