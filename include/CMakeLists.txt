# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_library(itlib INTERFACE)
add_library(itlib::itlib ALIAS itlib)

target_sources(itlib INTERFACE FILE_SET HEADERS FILES
    itlib/any.hpp
    itlib/atomic.hpp
    itlib/atomic_shared_ptr_storage.hpp
    itlib/data_mutex.hpp
    itlib/dynamic_bitset.hpp
    itlib/expected.hpp
    itlib/flat_map.hpp
    itlib/flat_set.hpp
    itlib/generator.hpp
    itlib/make_ptr.hpp
    itlib/mem_streambuf.hpp
    itlib/memory_view.hpp
    itlib/mutex.hpp
    itlib/pmr_allocator.hpp
    itlib/pod_vector.hpp
    itlib/poly_span.hpp
    itlib/qalgorithm.hpp
    itlib/rstream.hpp
    itlib/sentry.hpp
    itlib/shared_from.hpp
    itlib/small_vector.hpp
    itlib/span.hpp
    itlib/static_vector.hpp
    itlib/stride_span.hpp
    itlib/strutil.hpp
    itlib/tep_vector.hpp
    itlib/throw_ex.hpp
    itlib/time_t.hpp
    itlib/transparent_umap.hpp
    itlib/type_traits.hpp
    itlib/ufunction.hpp
    itlib/utility.hpp
)

install(TARGETS itlib
    EXPORT itlib-targets
    FILE_SET HEADERS
)
