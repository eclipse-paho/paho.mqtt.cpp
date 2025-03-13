# CMakeLists.txt
#
# CMake file for the Paho C++ core library.
#
# This is part of the Paho MQTT C++ client library.
#

#*******************************************************************************
# Copyright (c) 2016-2017, Guilherme Maciel Ferreira
# Copyright (c) 2017-2024, Frank Pagliughi
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
#     Guilherme Maciel Ferreira - initial version
#     Frank Pagliughi - made the shared library optional
#*******************************************************************************/

## --- Library dependencies ---

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(GenerateExportHeader)

## --- Use object library to optimize compilation ---

set(COMMON_SRC
    async_client.cpp
    client.cpp
    connect_options.cpp
    create_options.cpp    
    disconnect_options.cpp
    iclient_persistence.cpp
    message.cpp
    properties.cpp
    reason_code.cpp
    response_options.cpp
    server_response.cpp
    ssl_options.cpp
    string_collection.cpp
    token.cpp
    topic.cpp
    will_options.cpp
)

## --- Build the shared library, if requested ---

if(PAHO_BUILD_SHARED)
    message(STATUS "Creating shared library")

    ## Create the shared library
    add_library(paho-mqttpp3-shared SHARED ${COMMON_SRC})
    list(APPEND PAHO_CPP_TARGETS paho-mqttpp3-shared)

    ## Alias for subdirectory builds
    add_library(PahoMqttCpp::paho-mqttpp3-shared ALIAS paho-mqttpp3-shared)
    add_library(PahoMqttCpp::paho-mqttpp3 ALIAS paho-mqttpp3-shared)

    target_compile_definitions(paho-mqttpp3-shared PRIVATE PAHO_MQTTPP_EXPORTS)

    ## Add dependencies to the shared library
    target_link_libraries(paho-mqttpp3-shared PUBLIC
        ${PAHO_MQTT_C_LIB}
        Threads::Threads
        ${LIBS_SYSTEM}
    )

    ## set the shared library soname
    set_target_properties(paho-mqttpp3-shared PROPERTIES
        OUTPUT_NAME paho-mqttpp3
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    generate_export_header(paho-mqttpp3-shared
        BASE_NAME paho_mqttpp
        EXPORT_FILE_NAME ${PAHO_MQTTPP_GENERATED_DIR}/include/mqtt/export.h
    )
endif()

## --- Build static version of the library, if requested ---

if(PAHO_BUILD_STATIC)
    ## Create the static library
    add_library(paho-mqttpp3-static STATIC ${COMMON_SRC})
    list(APPEND PAHO_CPP_TARGETS paho-mqttpp3-static)

    ## Alias for subdirectory builds
    add_library(PahoMqttCpp::paho-mqttpp3-static ALIAS paho-mqttpp3-static)

    ## Add dependencies to the static library
    target_link_libraries(paho-mqttpp3-static PUBLIC 
        ${PAHO_MQTT_C_LIB}-static
        Threads::Threads
        ${LIBS_SYSTEM}
    )

    if(${PAHO_BUILD_SHARED})
        # This lib should configure for static exports
        target_compile_definitions(paho-mqttpp3-static PRIVATE PAHO_MQTTPP_STATIC)
        target_compile_definitions(paho-mqttpp3-static PRIVATE PAHO_MQTTPP_STATIC_DEFINE)
    else()
        # If no shared lib, make this one the default target
        add_library(PahoMqttCpp::paho-mqttpp3 ALIAS paho-mqttpp3-static)

        generate_export_header(paho-mqttpp3-static
            BASE_NAME paho_mqttpp
            EXPORT_FILE_NAME ${PAHO_MQTTPP_GENERATED_DIR}/include/mqtt/export.h
        )
    endif()

    ## Let the archive use the same base name as the shared lib on *nix systems
    if(UNIX)
        set_target_properties(paho-mqttpp3-static PROPERTIES OUTPUT_NAME paho-mqttpp3)
    endif()
endif()

## --- Make sure at least one target was specified ---

if(NOT PAHO_CPP_TARGETS)
    message(FATAL_ERROR "No build targets are specified")
endif()

## --- Set common properties, etc ---

include(GNUInstallDirs)

foreach(TARGET ${PAHO_CPP_TARGETS})
    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD ${PAHO_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    ## Build warnings
    target_compile_options(${TARGET} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W3>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wdocumentation>
        $<$<NOT:$<OR:$<CXX_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:Clang>>>:-Wall -Wextra>
    )

    target_include_directories(${TARGET} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    ## install the shared library
    install(TARGETS ${TARGET} EXPORT PahoMqttCpp
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach()
