# CMakeLists.txt
#
# CMake file for the Paho C++ example applications.
#
#*******************************************************************************
# This is part of the Paho MQTT C++ client library.
#
# Copyright (c) 2016-2024
# 
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v2.0
# and Eclipse Distribution License v1.0 which accompany this distribution.
# 
# The Eclipse Public License is available at
#   http://www.eclipse.org/legal/epl-v20.html
# and the Eclipse Distribution License is available at
#   http://www.eclipse.org/org/documents/edl-v10.php.
# 
# Contributors:
#   Guilherme Maciel Ferreira - initial version
#   Frank Pagliughi - Updates for new samples
#*******************************************************************************/

## --- Library dependencies ---

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# The example applications
set(EXECUTABLES
    async_publish
    async_publish_time
    async_subscribe
    async_subscribe_v5
    async_consume
    async_consume_v5
    async_message_consume
    async_message_consume_v5
    data_publish
    mqttpp_chat
    multithr_pub_sub
    pub_speed_test
    rpc_math_cli
    rpc_math_srvr
    server_props_v5
    sync_publish
    sync_consume
    sync_consume_v5
    sync_reconnect
    topic_publish
    ws_publish
    self_adaptive_publisher
)

# These will only be built if SSL selected
if(PAHO_WITH_SSL)
    set(SSL_EXECUTABLES ssl_publish)
endif()

## Build the example apps
foreach(EXECUTABLE ${EXECUTABLES} ${SSL_EXECUTABLES})
    add_executable(${EXECUTABLE} ${EXECUTABLE}.cpp)
    target_link_libraries(${EXECUTABLE} PahoMqttCpp::paho-mqttpp3)

    set_target_properties(${EXECUTABLE} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(PAHO_BUILD_SHARED)
        target_compile_definitions(${EXECUTABLE} PRIVATE PAHO_MQTTPP_IMPORTS)
    endif()
endforeach()

## Extra configuration for the SSL/TLS examples, if selected
foreach(EXECUTABLE ${SSL_EXECUTABLES})
    target_compile_definitions(${EXECUTABLE} PUBLIC OPENSSL)
endforeach()

## install binaries
include(GNUInstallDirs)

install(TARGETS ${EXECUTABLES} ${SSL_EXECUTABLES}
    EXPORT PahoMqttCppSamples
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


