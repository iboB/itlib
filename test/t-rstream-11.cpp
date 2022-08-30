#include <doctest/doctest.h>

#include <itlib/rstream.hpp>

#include <sstream>

TEST_SUITE_BEGIN("rstream");

const char* text = "1234567890abcdefghij";

TEST_CASE("no rebase")
{
    std::istringstream sin(text);

    char data[6] = "-----";
    const char* str = data;
    sin.read(data, 5);
    CHECK(std::string(str) == "12345");

    {
        itlib::rstream rin(sin);
        rin.read(data, 5);
        CHECK(std::string(str) == "67890");
    }

    sin.read(data, 5);
    CHECK(std::string(str) == "abcde");

    //{
    //    itlib::rstream rin(sin);
    //    char c;
    //    rin >> c;
    //    CHECK(c == 'f');
    //}
}

TEST_CASE("yes rebase")
{
    std::istringstream sin(text);

    char data[6] = "-----";
    const char* str = data;

    {
        itlib::redirect_rstream rin(sin, 5);
        rin.read(data, 5);
        CHECK(std::string(str) == "67890");

        //char c;
        //rin >> c;
        //CHECK(c == 'a');
    }

    sin.read(data, 5);
    CHECK(std::string(str) == "12345");
}