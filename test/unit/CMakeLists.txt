# CMakeLists.txt
#
# CMake file for the Catch2 unit tests in the Eclipse Paho C++ library.
#

#*******************************************************************************
# Copyright (c) 2019-2023 Frank Pagliughi <fpagliughi@mindspring.com>
#
#  All rights reserved. This program and the accompanying materials
#  are made available under the terms of the Eclipse Public License v2.0
#  and Eclipse Distribution License v1.0 which accompany this distribution. 
# 
#  The Eclipse Public License is available at 
#     http://www.eclipse.org/legal/epl-v20.html
#  and the Eclipse Distribution License is available at 
#    http://www.eclipse.org/org/documents/edl-v10.php.
# 
#  Contributors:
#     Frank Pagliughi - Initial implementation
#*******************************************************************************/

# --- Find Catch2 and figure out which major version ---

find_package(Catch2 REQUIRED)

message(STATUS "Found Catch2 v${Catch2_VERSION}")

if (Catch2_VERSION VERSION_LESS "2.0")
    message(FATAL "Catch2 v2.0 or greater required for tests")
endif()

# --- Executables ---

add_executable(unit_tests unit_tests.cpp
    test_async_client.cpp
    test_buffer_ref.cpp
    test_client.cpp
    test_connect_options.cpp
    test_create_options.cpp
    test_disconnect_options.cpp
    test_exception.cpp
    test_message.cpp
    test_persistence.cpp
    test_properties.cpp
    test_response_options.cpp
    test_string_collection.cpp
    test_subscribe_options.cpp
    test_thread_queue.cpp
    test_token.cpp
    test_topic.cpp
    test_topic_matcher.cpp
    test_will_options.cpp
)

if(PAHO_WITH_SSL)
    target_sources(unit_tests PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_ssl_options.cpp
    )
endif()

set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if (Catch2_VERSION VERSION_LESS "3.0")
    target_compile_definitions(unit_tests PUBLIC CATCH2_V2)
endif()

# --- Link for executables ---

target_link_libraries(unit_tests
    Catch2::Catch2
    PahoMqttCpp::paho-mqttpp3
)

if(PAHO_BUILD_SHARED)
    target_compile_definitions(unit_tests PUBLIC PAHO_MQTTPP_IMPORTS)

    if(MSVC AND PAHO_BUILD_STATIC)
        target_link_libraries(unit_tests ${LIBS_SYSTEM})
    endif()
endif()

# On Windows with shared libraries, copy DLLs next to the test binary so that
# it can run (both for catch_discover_tests and when running ctest directly).
if(WIN32 AND PAHO_BUILD_SHARED)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
        add_custom_command(TARGET unit_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:unit_tests>
                $<TARGET_RUNTIME_DLLS:unit_tests>
            COMMAND_EXPAND_LISTS
        )
    else()
        # Fallback for CMake < 3.21: copy known DLL targets manually.
        set(_paho_dlls $<TARGET_FILE:paho-mqttpp3-shared>)
        if(PAHO_WITH_MQTT_C)
            if(PAHO_WITH_SSL)
                list(APPEND _paho_dlls $<TARGET_FILE:paho-mqtt3as>)
            else()
                list(APPEND _paho_dlls $<TARGET_FILE:paho-mqtt3a>)
            endif()
        else()
            message(WARNING
                "CMake 3.21+ is recommended to automatically copy all runtime "
                "DLL dependencies for unit tests. With CMake ${CMAKE_VERSION}, "
                "only the paho-mqttpp3 DLL will be copied automatically. "
                "Ensure the paho MQTT C DLLs are in PATH for tests to run."
            )
        endif()
        add_custom_command(TARGET unit_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_paho_dlls}
                $<TARGET_FILE_DIR:unit_tests>
        )
    endif()
endif()

include(CTest)
include(Catch)

catch_discover_tests(unit_tests)

