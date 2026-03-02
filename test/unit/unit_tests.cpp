// unit_tests.cpp
//
// Main for the Catch2 unit tests for the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2019-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - initial implementation
 *******************************************************************************/

// This seems to be required, at least for MSVS 2015 on Win7,
// using Catch2 v2.9.2
#if defined(_WIN32)
    #define CATCH_CONFIG_DISABLE_EXCEPTIONS
#endif

#define CATCH_CONFIG_RUNNER
#include "catch2_version.h"

int main(int argc, char* argv[])
{
    // global setup...

    int result = Catch::Session().run(argc, argv);

    // global clean-up...

    return result;
}
