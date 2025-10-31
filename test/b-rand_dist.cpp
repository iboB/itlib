#include <itlib/rand_dist.hpp>
#include <random>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#define uauto [[maybe_unused]] auto

struct mean {
    double val = 0.0;
    size_t count = 0;

    template <typename T>
    mean& operator+=(T v) {
        val += double(v);
        ++count;
        return *this;
    }

    uintptr_t get(double scale = 100) const {
        return uintptr_t(std::round(scale * val / double(count)));
    }
};

template <typename Rng, typename Dist>
void bench_int_dist(picobench::state& s) {
    Rng rng(9857);
    Dist dist(-100, 100);
    mean m;
    for (uauto _ : s) {
        m += dist(rng);
    }
    s.set_result(m.get(10));
}

template <typename Rng, typename Dist>
void bench_real_dist(picobench::state& s) {
    Rng rng(12345);
    Dist dist(-2, 3);
    mean m;
    for (uauto _ : s) {
        m += dist(rng);
    }
    s.set_result(m.get());
}

template <typename Rng, typename Dist>
void bench_real_dist_01(picobench::state& s) {
    Rng rng(12345);
    mean m;
    for (uauto _ : s) {
        m += Dist::draw_01(rng);
    }
    s.set_result(m.get());
}

template <typename Rng, typename F>
void bench_std_real_dist_01(picobench::state& s) {
    Rng rng(12345);
    std::uniform_real_distribution<F> dist(0.0, 1.0);
    mean m;
    for (uauto _ : s) {
        m += dist(rng);
    }
    s.set_result(m.get());
}

template <typename T>
using int_itlib = itlib::uniform_int_distribution<T>;
template <typename T>
using int_std = std::uniform_int_distribution<T>;
template <typename T>
using real_fast = itlib::fast_uniform_real_distribution<T>;
template <typename T>
using real_std = std::uniform_real_distribution<T>;

template <typename Rng>
void add_int_benchmarks(picobench::local_runner& r) {
    r.add_benchmark("itlib int32", bench_int_dist<Rng, int_itlib<int32_t>>);
    r.add_benchmark("std int32", bench_int_dist<Rng, int_std<int32_t>>);
    r.add_benchmark("itlib int64", bench_int_dist<Rng, int_itlib<int64_t>>);
    r.add_benchmark("std int64", bench_int_dist<Rng, int_std<int64_t>>);
}

template <typename Rng>
void add_real_benchmarks(picobench::local_runner& r) {
    r.add_benchmark("fast float", bench_real_dist<Rng, real_fast<float>>);
    r.add_benchmark("fast double", bench_real_dist<Rng, real_fast<double>>);
    r.add_benchmark("std float", bench_real_dist<Rng, real_std<float>>);
    r.add_benchmark("std double", bench_real_dist<Rng, real_std<double>>);
}

int main(int argc, char* argv[]) {
    picobench::local_runner r;

    r.set_suite("int dist mt19937");
    add_int_benchmarks<std::mt19937>(r);

    r.set_suite("int dist minstd_rand");
    add_int_benchmarks<std::minstd_rand>(r);

    r.set_suite("real dist mt19937");
    add_real_benchmarks<std::mt19937>(r);

    r.set_suite("real dist mt19937 01");
    r.add_benchmark("fast float", bench_real_dist_01<std::mt19937, real_fast<float>>);
    r.add_benchmark("fast double", bench_real_dist_01<std::mt19937, real_fast<double>>);
    r.add_benchmark("std float", bench_std_real_dist_01<std::mt19937, float>);
    r.add_benchmark("std double", bench_std_real_dist_01<std::mt19937, double>);

    r.set_suite("real dist mt19937_64");
    add_real_benchmarks<std::mt19937_64>(r);

    r.set_suite("real dist minstd_rand");
    add_real_benchmarks<std::minstd_rand>(r);

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.set_default_state_iterations({1'000'000});
    r.parse_cmd_line(argc, argv);
    return r.run();
}
