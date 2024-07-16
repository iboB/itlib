// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/utility.hpp>

#include <string>

int main() {
    const std::string cstr = "asdf";
    // build error: cannot force_move a const object
    auto fail = itlib::force_move(cstr);
}
