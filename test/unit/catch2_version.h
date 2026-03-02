// catch2_version.h
//
// Figure out which Catch2 header file to locad by version.
//

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
 *******************************************************************************/

#ifndef __catch2_version_h
#define __catch2_version_h

#ifdef CATCH2_V2
    // For Catch2 v2.x
    #include "catch2/catch.hpp"
#else
    // For Catch2 v3.x (and beyond?)
    #include "catch2/catch_all.hpp"
#endif

#endif  // __catch2_version_h
