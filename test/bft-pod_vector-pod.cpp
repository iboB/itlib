// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/pod_vector.hpp>
#include <string>

// build error: itlib::pod_vector with non-trivial type
itlib::pod_vector<std::string> vec;
