#include <doctest/doctest.h>

#include <itlib/mem_streambuf.hpp>

#include <ostream>
#include <istream>
#include <string>

#include <vector>

class istr : public std::istream {
public:
    istr(const char* text) : std::istream(&buf) {
        buf.reset(text, strlen(text));
    }
    itlib::mem_istreambuf<char> buf;
};

TEST_CASE("in") {
    {
        istr in("asd");
        CHECK(in.get() == 'a');
        CHECK(in.get() == 's');
        CHECK(in.get() == 'd');
        CHECK(in.get() == istr::traits_type::eof());
        CHECK(in.get() == istr::traits_type::eof());
        in.clear();
        in.seekg(-2, std::ios::cur);
        CHECK(in.get() == 's');
        in.seekg(-3, std::ios::end);
        CHECK(in.get() == 'a');
        in.seekg(2, std::ios::beg);
        CHECK(in.get() == 'd');
        in.seekg(0);
        CHECK(in.get() == 'a');
    }

    {
        istr in("l1\nl2\r\nl3");
        std::string line;
        std::getline(in, line);
        CHECK(line == "l1");
        std::getline(in, line);
        CHECK(line == "l2\r"); // treating this as binary on windows!
        std::getline(in, line);
        CHECK(line == "l3");
    }

    {
        istr in("asdf");
        char foo[100] = {};
        in.read(foo, 2);
        CHECK(strcmp(foo, "as") == 0);
        CHECK(in.tellg() == 2);
        in.seekg(0);
        in.read(foo, 90);
        CHECK(strcmp(foo, "asdf") == 0);
        CHECK(in.gcount() == 4);
        CHECK(in.eof());
    }
}

class ostr : public std::ostream {
public:
    ostr(size_t reserve = 0) : std::ostream(&buf), buf(reserve) {}
    itlib::mem_ostreambuf<std::vector<char>> buf;
};

TEST_CASE("out") {
    {
        ostr out;
        out << "hello";
        out << "world";
        auto v = out.buf.get_container();
        REQUIRE(v.size() == 10);
        CHECK(memcmp(v.data(), "helloworld", 10) == 0);
    }

    {
        ostr out;
        out << 123;
        auto& v = out.buf.peek_container();
        CHECK(v[0] == '1');
        CHECK(v[1] == '2');
        CHECK(v[2] == '3');
        auto cv = out.buf.get_container();
        CHECK(cv.size() == 3);
        CHECK(cv[0] == '1');
        CHECK(v.empty());
        out << 2 << "x";
        CHECK(v[0] == '2');
        CHECK(v[1] == 'x');
        cv = out.buf.get_container();
        CHECK(cv.size() == 2);
    }

    {
        ostr out;
        out.buf.clear();
        out << 'x' << std::endl;
        auto& v = out.buf.peek_container();
        CHECK(v[0] == 'x');
        CHECK(v[1] == '\n');
        CHECK(v.size() == 2);
        out.buf.clear();
        out << 'y';
        auto cv = out.buf.get_container();
        CHECK(cv.size() == 1);
        CHECK(cv.front() == 'y');
    }
}

#include <itlib/static_vector.hpp>

TEST_CASE("out with static_vector")
{
    itlib::mem_ostreambuf<itlib::static_vector<char, 1024>> buf;
    std::ostream out(&buf);

    out << "hello";
    out << "world";

    auto v = buf.get_container();
    REQUIRE(v.size() == 10);
    CHECK(memcmp(v.data(), "helloworld", 10) == 0);
}

