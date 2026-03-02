# Eclipse Paho MQTT C++ Client Library
# Coding Styles

This document describes the coding style and language conventions used by the Eclipse Paho C++ Client Library.

## Language Standard

The Paho C++ library uses Modern C++, adhering to the C++11 standard.

Nothing in the library should prevent it from being built with a compiler adhering to a newer standard such as C++14 or C++17, but at this point the library itself doesn't use any newer language feature than C++11. Unfortunately, compilers vary wildly from version to version and platform to platform. So this is a constant struggle to keep true.

Adherence to the base C++11 standard may change at some point in the future. Newer versions of the library will likely start using newer language features some time in the future as they are more widely adapted by compilers and projects. The next logical jump would probably be to C++17.

## Naming Convention

The Paho C++ library attempts to follow the naming conventions of the C++ standard library as much as possible, as this seems the most definitive standard for the language.

 - Class names are lower snake case:  *classes_like_this*
 - Function names are lower snake case:  *functions_like_this*
 - Variable names are lower camel case:  *varsLikeThis*
 - Class members are lower camel case with a trailing underscore: *memVarsLikeThis_*
 - Constants are all caps: *CONSTANTS_LIKE_THIS*

## Format Conventions

The top-level project directory contains a _.editorconfig_ file with some basic hints as to formatting conventions for source files.

A few minutes looking through the existing sources will reveal the basic styles used. Pull Requests should generally try to fit in to the existing style and not try to impose new ones.

At some point in the future, a formatter may be added to the project (probably Clang format), but until then, any sources that can be fixed up with an automated code generator or beautifier would generally be accepted into the project. But at some point soon after would then be reformat to fit into the overall conventions.