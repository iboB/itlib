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
struct dist {
    using fast = itlib::fast_uniform_real_distribution<T>;
    using std = std::uniform_real_distribution<T>;
};

int main(int argc, char* argv[]) {
    picobench::local_runner r;

    r.set_suite("real dist mt19937");
    r.add_benchmark("fast float", bench_real_dist<std::mt19937, dist<float>::fast>);
    r.add_benchmark("fast double", bench_real_dist<std::mt19937, dist<double>::fast>);
    r.add_benchmark("std float", bench_real_dist<std::mt19937, dist<float>::std>);
    r.add_benchmark("std double", bench_real_dist<std::mt19937, dist<double>::std>);

    r.set_suite("real dist mt19937 01");
    r.add_benchmark("fast float 01", bench_real_dist_01<std::mt19937, dist<float>::fast>);
    r.add_benchmark("fast double 01", bench_real_dist_01<std::mt19937, dist<double>::fast>);
    r.add_benchmark("std float 01", bench_std_real_dist_01<std::mt19937, float>);
    r.add_benchmark("std double 01", bench_std_real_dist_01<std::mt19937, double>);

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.set_default_state_iterations({1'000'000});
    r.parse_cmd_line(argc, argv);
    return r.run();
}
