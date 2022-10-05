## itlib unit tests

The unit tests for a lib are per a C++ standard. The number at the end of the filename shows the standard in question.

Most tests are for C++11, but if a library offers extra features for other standards, it also has tests for them.

The "itlib" tests check that the compiler meets the requirements for the libraries and tests to run adequately.

Tests prefixed with "bft" are build failure tests. They check for expected compilation errors.

Using [doctest](https://github.com/onqtam/doctest) through [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)
