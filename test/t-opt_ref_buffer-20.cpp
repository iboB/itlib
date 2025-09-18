// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/opt_ref_buffer.hpp>
#include <doctest/doctest.h>
#include <itlib/pod_vector.hpp>
#include <array>
#include <cstring>
#include <numeric>

TEST_CASE("empty") {
    itlib::opt_ref_buffer b;
    CHECK(b.data() == nullptr);
    CHECK(b.size() == 0);
    CHECK(b.size_bytes() == 0);
    CHECK(b.empty());
    CHECK(b.span().data() == nullptr);
    CHECK(b.span().size() == 0);
    CHECK(b.own().index() == 0);
    CHECK_FALSE(b.owns_data());
    CHECK(std::holds_alternative<std::monostate>(b.own()));

    itlib::opt_ref_buffer_t<int> ib;
    CHECK(ib.data() == nullptr);
    CHECK(ib.size() == 0);

    itlib::const_opt_ref_buffer cb;
    CHECK(cb.data() == nullptr);
    CHECK(cb.size() == 0);
}

TEST_CASE("take") {
    std::vector<int> ints = {1, 2, 3, 4, 5};
    auto ints_ptr = ints.data();

    auto b = itlib::opt_ref_buffer::take(std::move(ints));
    CHECK(b.owns_data());
    CHECK((void*)b.data() == ints_ptr);

    auto span = b.span();
    CHECK(span.size() == sizeof(int) * 5);
}

TEST_CASE("noexcept guarantees") {
    SUBCASE("basic operations") {
        static_assert(std::is_nothrow_default_constructible_v<itlib::opt_ref_buffer>);
        static_assert(std::is_nothrow_move_constructible_v<itlib::opt_ref_buffer>);
        static_assert(std::is_nothrow_move_assignable_v<itlib::opt_ref_buffer>);

        static_assert(std::is_nothrow_default_constructible_v<itlib::opt_ref_buffer_t<int>>);
        static_assert(std::is_nothrow_move_constructible_v<itlib::opt_ref_buffer_t<int>>);
        static_assert(std::is_nothrow_move_assignable_v<itlib::opt_ref_buffer_t<int>>);

        static_assert(std::is_nothrow_default_constructible_v<itlib::const_opt_ref_buffer>);
        static_assert(std::is_nothrow_move_constructible_v<itlib::const_opt_ref_buffer>);
        static_assert(std::is_nothrow_move_assignable_v<itlib::const_opt_ref_buffer>);
    }

    SUBCASE("query operations") {
        itlib::opt_ref_buffer b;
        static_assert(noexcept(b.span()));
        static_assert(noexcept(b.data()));
        static_assert(noexcept(b.owns_data()));
        // Note: own() returns a reference to variant, which might not be noexcept
    }

    SUBCASE("ref operations for congruent types") {
        std::vector<int> vec = {1, 2, 3};
        static_assert(noexcept(itlib::opt_ref_buffer_t<int>::ref(vec)));
    }

    SUBCASE("byte vector construction") {
        static_assert(noexcept(itlib::opt_ref_buffer(std::vector<std::byte>{})));
    }
}

TEST_CASE("congruency") {
    SUBCASE("congruent types - same size") {
        std::vector<int> ints = {0x12345678, 0x3ABCDEF0};
        auto b = itlib::opt_ref_buffer_t<int>::ref(ints);
        CHECK(b.size() == 2);
        CHECK(b.size_bytes() == sizeof(int) * 2);
        CHECK(b.data()[0] == 0x12345678);
        CHECK(b.data()[1] == 0x3ABCDEF0);
    }

    SUBCASE("congruent types - byte to larger") {
        std::vector<std::byte> bytes(8);
        std::memset(bytes.data(), 0xAB, 8);

        auto b = itlib::opt_ref_buffer_t<uint32_t>::ref(bytes);
        CHECK(b.size() == 2);

        // Check that we can access as uint32_t
        auto* ptr = reinterpret_cast<uint32_t*>(bytes.data());
        CHECK(b.data()[0] == ptr[0]);
        CHECK(b.data()[1] == ptr[1]);
    }

    SUBCASE("congruent types - uint16_t to uint32_t") {
        std::vector<uint16_t> shorts = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
        auto b = itlib::opt_ref_buffer_t<uint32_t>::ref(shorts);
        CHECK(b.size() == 2);
    }

    SUBCASE("non-congruent types should throw") {
        std::vector<char> chars(7); // 7 bytes cannot fit evenly into int (4 bytes)
        CHECK_THROWS_AS(itlib::opt_ref_buffer_t<int>::ref(chars), std::runtime_error);

        std::vector<uint16_t> shorts(3); // 6 bytes cannot fit evenly into int (4 bytes)
        CHECK_THROWS_AS(itlib::opt_ref_buffer_t<int>::ref(shorts), std::runtime_error);
    }
}

TEST_CASE("copying") {
    SUBCASE("copy vector") {
        std::vector<int> original = {1, 2, 3, 4, 5};
        auto original_ptr = original.data();

        auto b = itlib::opt_ref_buffer_t<int>::copy(original);
        CHECK(b.owns_data());
        CHECK(b.data() != original_ptr); // Different pointer - it's a copy
        CHECK(b.size() == 5);
        CHECK(b.data()[0] == 1);
        CHECK(b.data()[4] == 5);

        // Original should be unchanged
        CHECK(original.size() == 5);
        CHECK(original[0] == 1);
    }

    SUBCASE("copy string") {
        std::string original = "Hello, World!";

        auto b = itlib::opt_ref_buffer::copy(original);
        CHECK(b.owns_data());
        // Note: for small strings, pointer might be the same due to SSO, but data is still copied
        CHECK(b.size() == original.size());

        // Verify content
        auto span_chars = reinterpret_cast<const char*>(b.data());
        CHECK(std::memcmp(span_chars, original.data(), original.size()) == 0);
    }

    SUBCASE("copy array") {
        std::array<int, 4> arr = {10, 20, 30, 40};
        auto b = itlib::opt_ref_buffer_t<int>::copy(arr);
        CHECK(b.owns_data());
        CHECK(b.data() != arr.data()); // Different pointer
        CHECK(b.size() == 4);
        CHECK(b.data()[0] == 10);
        CHECK(b.data()[3] == 40);
    }
}

TEST_CASE("moving") {
    SUBCASE("move vector") {
        std::vector<int> vec = {1, 2, 3, 4, 5};
        auto original_ptr = vec.data();

        auto b = itlib::opt_ref_buffer_t<int>::take(std::move(vec));
        CHECK(b.owns_data());
        CHECK(b.data() == original_ptr); // Same pointer - moved
        CHECK(b.size() == 5);
        CHECK(b.data()[0] == 1);
        CHECK(b.data()[4] == 5);

        // Original vector should be empty after move
        CHECK(vec.empty());
    }

    SUBCASE("move string") {
        std::string str = "This is a longer string to avoid small string optimization";
        auto original_ptr = str.data();

        auto b = itlib::opt_ref_buffer::take(std::move(str));
        CHECK(b.owns_data());
        CHECK(b.data() == reinterpret_cast<std::byte*>(original_ptr)); // Same pointer for long strings

        // Original string should be empty after move
        CHECK(str.empty());
    }

    SUBCASE("move buffer to buffer") {
        std::vector<int> vec = {1, 2, 3};
        auto b1 = itlib::opt_ref_buffer_t<int>::take(std::move(vec));
        auto original_ptr = b1.data();

        auto b2 = itlib::opt_ref_buffer_t<int>(std::move(b1));
        CHECK(b2.owns_data());
        CHECK(b2.data() == original_ptr);
        CHECK(b2.size() == 3);

        // b1 should be empty after move
        CHECK(b1.data() == nullptr);
        CHECK(b1.size() == 0);
        CHECK_FALSE(b1.owns_data());
    }

    SUBCASE("move assignment") {
        std::vector<int> vec1 = {1, 2, 3};
        std::vector<int> vec2 = {4, 5, 6, 7};

        auto b1 = itlib::opt_ref_buffer_t<int>::take(std::move(vec1));
        auto b2 = itlib::opt_ref_buffer_t<int>::take(std::move(vec2));
        auto ptr2 = b2.data();

        b1 = std::move(b2);
        CHECK(b1.owns_data());
        CHECK(b1.data() == ptr2);
        CHECK(b1.size() == 4);
        CHECK(b1.data()[3] == 7);

        // b2 should be empty after move
        CHECK(b2.data() == nullptr);
        CHECK_FALSE(b2.owns_data());
    }
}

TEST_CASE("referencing") {
    SUBCASE("reference vector") {
        std::vector<int> vec = {1, 2, 3, 4, 5};
        auto original_ptr = vec.data();

        auto b = itlib::opt_ref_buffer_t<int>::ref(vec);
        CHECK_FALSE(b.owns_data());
        CHECK(b.data() == original_ptr); // Same pointer - referencing
        CHECK(b.size() == 5);
        CHECK(b.data()[0] == 1);
        CHECK(b.data()[4] == 5);

        // Modify through buffer
        b.data()[0] = 10;
        CHECK(vec[0] == 10); // Original vector should be modified
    }

    SUBCASE("reference string") {
        std::string str = "Hello";

        auto b = itlib::opt_ref_buffer::ref(str);
        CHECK_FALSE(b.owns_data());
        CHECK(b.data() == reinterpret_cast<std::byte*>(str.data()));
        CHECK(b.size() == str.size());
    }

    SUBCASE("reference string literal") {
        auto b = itlib::const_opt_ref_buffer::ref("Hello");
        CHECK_FALSE(b.owns_data());
        CHECK(b.size() == 5);
        CHECK(std::string_view(reinterpret_cast<const char*>(b.data()), b.size()) == "Hello");
    }

    SUBCASE("reference string view") {
        const wchar_t* wstr = L"Buenos dias";
        auto b = itlib::const_opt_ref_buffer::ref(std::wstring_view(wstr));
        CHECK_FALSE(b.owns_data());
        CHECK(b.size() == 11 * sizeof(wchar_t));
        CHECK((void*)b.data() == wstr);
    }

    SUBCASE("const reference") {
        const std::vector<int> vec = {1, 2, 3};
        auto b = itlib::opt_ref_buffer_t<const int>::ref(vec);
        CHECK_FALSE(b.owns_data());
        CHECK(b.data() == vec.data());
        CHECK(b.size() == 3);
        CHECK(b.data()[0] == 1);
    }
}

TEST_CASE("pointer preservation") {
    SUBCASE("vector pointer preservation on move") {
        std::vector<int> vec(1000); // Large enough to avoid any optimizations
        std::iota(vec.begin(), vec.end(), 0);
        auto original_ptr = vec.data();

        auto b = itlib::opt_ref_buffer_t<int>::take(std::move(vec));
        CHECK(b.data() == original_ptr);

        // Move buffer to another buffer
        auto b2 = itlib::opt_ref_buffer_t<int>(std::move(b));
        CHECK(b2.data() == original_ptr);
    }

    SUBCASE("long string pointer preservation") {
        // Use a long string to avoid small string optimization
        std::string long_str(1000, 'A');
        auto original_ptr = long_str.data();

        auto b = itlib::opt_ref_buffer::take(std::move(long_str));
        CHECK(b.data() == reinterpret_cast<std::byte*>(original_ptr));

        // Move to another buffer
        auto b2 = itlib::opt_ref_buffer(std::move(b));
        CHECK(b2.data() == reinterpret_cast<std::byte*>(original_ptr));
    }

    SUBCASE("small string handling") {
        // Small strings might use SSO, so pointer might change on move
        std::string small_str = "small";

        auto b = itlib::opt_ref_buffer::take(std::move(small_str));
        CHECK(b.owns_data());
        CHECK(b.size() == 5);

        // Content should be preserved regardless of pointer
        auto chars = reinterpret_cast<const char*>(b.data());
        CHECK(std::memcmp(chars, "small", 5) == 0);

        // Move to another buffer - content should still be preserved
        auto b2 = itlib::opt_ref_buffer(std::move(b));
        CHECK(b2.owns_data());
        CHECK(b2.size() == 5);
        chars = reinterpret_cast<const char*>(b2.data());
        CHECK(std::memcmp(chars, "small", 5) == 0);
    }

    SUBCASE("reference pointer preservation") {
        std::vector<int> vec = {1, 2, 3, 4, 5};

        auto b = itlib::opt_ref_buffer_t<int>::ref(vec);
        CHECK(b.data() == vec.data());

        // Even after moving the buffer, it should still reference original data
        auto b2 = itlib::opt_ref_buffer_t<int>(std::move(b));
        CHECK(b2.data() == vec.data());
        CHECK_FALSE(b2.owns_data()); // Still a reference, not owning
    }
}

TEST_CASE("type conversions") {
    SUBCASE("non-const to const conversion") {
        std::vector<int> vec = {1, 2, 3};
        auto b = itlib::opt_ref_buffer_t<int>::take(std::move(vec));
        auto original_ptr = b.data();

        auto const_b = itlib::opt_ref_buffer_t<const int>(std::move(b));
        CHECK(const_b.data() == original_ptr);
        CHECK(const_b.owns_data());
        CHECK(const_b.size() == 3);
        CHECK(const_b.data()[0] == 1);
    }

    SUBCASE("type reinterpretation") {
        std::vector<uint32_t> ints = {0x12345678, 0x9ABCDEF0};
        auto b = itlib::opt_ref_buffer_t<uint32_t>::take(std::move(ints));

        auto byte_b = itlib::opt_ref_buffer_t<std::byte>(std::move(b));
        CHECK(byte_b.owns_data());
        CHECK(byte_b.size() == 8); // 2 * sizeof(uint32_t)
    }
}

TEST_CASE("string literal and string_view construction") {
    SUBCASE("string literal") {
        auto b = itlib::opt_ref_buffer("Hello");
        CHECK(b.owns_data());
        CHECK(b.size() == 5);

        auto chars = reinterpret_cast<const char*>(b.data());
        CHECK(std::memcmp(chars, "Hello", 5) == 0);
    }

    SUBCASE("string_view") {
        std::string source = "Hello, World!";
        std::string_view sv(source.data() + 7, 5); // "World"

        auto b = itlib::opt_ref_buffer(sv);
        CHECK(b.owns_data());
        CHECK(b.size() == 5);

        auto chars = reinterpret_cast<const char*>(b.data());
        CHECK(std::memcmp(chars, "World", 5) == 0);
    }
}

TEST_CASE("edge cases") {
    SUBCASE("empty containers") {
        std::vector<int> empty_vec;
        auto b = itlib::opt_ref_buffer::take(std::move(empty_vec));
        CHECK_FALSE(b.owns_data());
        CHECK(b.empty());

        std::string empty_str;
        b = itlib::opt_ref_buffer::copy(empty_str);
        CHECK_FALSE(b.owns_data());
        CHECK(b.empty());

        std::vector<std::byte> empty_bytes;
        b = itlib::opt_ref_buffer::take(std::move(empty_bytes));
        CHECK_FALSE(b.owns_data());
        CHECK(b.empty());
    }

    SUBCASE("single element") {
        std::vector<int> single = {42};
        auto original_ptr = single.data();

        auto b = itlib::opt_ref_buffer_t<int>::take(std::move(single));
        CHECK(b.owns_data());
        CHECK(b.data() == original_ptr);
        CHECK(b.size() == 1);
        CHECK(b.data()[0] == 42);
    }

    SUBCASE("self move assignment") {
        std::vector<int> vec = {1, 2, 3};
        auto b = itlib::opt_ref_buffer_t<int>::take(std::move(vec));
        auto original_ptr = b.data();
#if !defined(_MSC_VER)
#   pragma GCC diagnostic ignored "-Wself-move"
#endif
        b = std::move(b); // Self assignment
        CHECK(b.data() == original_ptr); // Should remain unchanged
        CHECK(b.owns_data());
        CHECK(b.size() == 3);
    }
}
