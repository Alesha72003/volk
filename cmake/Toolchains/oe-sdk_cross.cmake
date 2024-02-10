#
# Copyright 2014 Free Software Foundation, Inc.
#
# This file is part of VOLK
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

set(CMAKE_SYSTEM_NAME Linux)
#set( CMAKE_C_COMPILER  $ENV{CC} )
#set( CMAKE_CXX_COMPILER  $ENV{CXX} )
string(REGEX MATCH "sysroots/([a-zA-Z0-9]+)" CMAKE_SYSTEM_PROCESSOR
             $ENV{SDKTARGETSYSROOT})
string(REGEX REPLACE "sysroots/" "" CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
set(CMAKE_CXX_FLAGS
    $ENV{CXXFLAGS}
    CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS
    $ENV{CFLAGS}
    CACHE STRING "" FORCE) #same flags for C sources
set(CMAKE_LDFLAGS_FLAGS
    ${CMAKE_CXX_FLAGS}
    CACHE STRING "" FORCE) #same flags for C sources
set(CMAKE_LIBRARY_PATH ${OECORE_TARGET_SYSROOT}/usr/lib)
set(CMAKE_FIND_ROOT_PATH $ENV{OECORE_TARGET_SYSROOT} $ENV{OECORE_NATIVE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(ORC_INCLUDE_DIRS $ENV{OECORE_TARGET_SYSROOT}/usr/include/orc-0.4)
set(ORC_LIBRARY_DIRS $ENV{OECORE_TARGET_SYSROOT}/usr/lib)
