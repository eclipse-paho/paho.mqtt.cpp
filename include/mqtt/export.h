/////////////////////////////////////////////////////////////////////////////
/// @file export.h
/// Library symbol export definitions, primarily for Windows MSVC DLL's
/// @date November 20, 2023
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2023 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *    Frank Pagliughi - MQTT v5 support
 *******************************************************************************/

#ifndef __mqtt_export_h
#define __mqtt_export_h

#if defined(_WIN32) && defined(_MSC_VER)
    #if defined(PAHO_MQTTPP_EXPORTS)
        #define PAHO_MQTTPP_EXPORT __declspec(dllexport)
    #elif defined(PAHO_MQTTPP_IMPORTS)
        #define PAHO_MQTTPP_EXPORT __declspec(dllimport)
    #else
        #define PAHO_MQTTPP_EXPORT
    #endif
#else
    #if defined(PAHO_MQTTPP_EXPORTS)
        #define PAHO_MQTTPP_EXPORT __attribute__((visibility("default")))
    #else
        #define PAHO_MQTTPP_EXPORT
    #endif
#endif

#endif  // __mqtt_export_h
