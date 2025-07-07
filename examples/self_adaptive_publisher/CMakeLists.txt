# CMakeLists.txt for self_adaptive_publisher
#
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

# The self_adaptive_publisher executable
add_executable(self_adaptive_publisher 
    self_adaptive_publisher.cpp
    broker_list_manager.cpp
    broker_monitor_thread.cpp
    mqtt_manager.cpp
)

target_link_libraries(self_adaptive_publisher 
    paho-mqttpp3
    paho-mqtt3a
    Threads::Threads
)

set_target_properties(self_adaptive_publisher PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(PAHO_BUILD_SHARED)
    target_compile_definitions(self_adaptive_publisher PRIVATE PAHO_MQTTPP_IMPORTS)
endif()

## install binary
include(GNUInstallDirs)

install(TARGETS self_adaptive_publisher
    EXPORT PahoMqttCppSamples
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 