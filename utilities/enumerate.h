#pragma once

#include <cstdint>
#include <utility>
#include "generator_iterator.h"

namespace {
    ////////////////////////////////////////////////////////////////////////////////
    // Forward Declarations
    ////////////////////////////////////////////////////////////////////////////////
    template<class UnderlyingIterator>
    struct EnumerateState;

    ////////////////////////////////////////////////////////////////////////////////
    // enumerate - This class behaves like a Generator, but does not inherit from
    // Generator because of a clang bug related to rvalue reference lifetime
    // extensions in base classes.  So the Generator behavior is emulated instead.
    ////////////////////////////////////////////////////////////////////////////////
    template<class Iterable>
    class enumerate {
    public:
        //------------------------------------------------------------------------------
        // Initialization variables - Can only be set via aggregate initialization
        //------------------------------------------------------------------------------
        Iterable iterable_;
        int64_t starting_idx_ = 0;

    public:
        //------------------------------------------------------------------------------
        // State of the generator - It's public to allow for aggregate initialization of
        // the above variables, but this should be treated as private.
        //------------------------------------------------------------------------------
        using State = EnumerateState<decltype(std::begin(iterable_))>;
        State state_{starting_idx_, std::begin(iterable_)};

    public:
        //------------------------------------------------------------------------------
        // Implicit Constructors - All Deleted
        //------------------------------------------------------------------------------
        constexpr enumerate() = delete;
        constexpr enumerate(const enumerate&) = delete;
        constexpr enumerate(enumerate&&) = delete;
        constexpr enumerate& operator=(const enumerate&) = delete;
        constexpr enumerate& operator=(enumerate&&) = delete;

    public:
        //------------------------------------------------------------------------------
        // Begin and End - Enables usage in ranged-for
        //------------------------------------------------------------------------------
        using Iterator = utilities::intern::GeneratorIterator<enumerate>;
        constexpr Iterator begin() { return Iterator{*this}; }
        constexpr utilities::intern::GeneratorEnd end() { return {}; }

    public:
        //------------------------------------------------------------------------------
        // operators - Emulating the generator class's operators
        //------------------------------------------------------------------------------
        constexpr State& operator*() { return state_; }
        constexpr enumerate& operator++() { ++state_.idx_; ++state_.iter_; return *this; }
        constexpr explicit operator bool() const { return state_.iter_ != std::end(iterable_); }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Deduction guides
    //      enumerate{some_iterable}
    //      enumerate{some_iterable, starting_index}
    ////////////////////////////////////////////////////////////////////////////////
    template<class Iterable>
    enumerate(Iterable&& iterable) -> enumerate<decltype(iterable)>;

    template<class Iterable>
    enumerate(Iterable&& iterable, int64_t) -> enumerate<decltype(iterable)>;

    // Clang and MSVC do not seem to be able to support this for various reasons at this time
#ifdef __GNUC__
#ifndef __clang__
    template<class ValueType> enumerate(std::initializer_list<ValueType>&& iterable) -> enumerate<decltype(iterable)>;
    template<class ValueType> enumerate(std::initializer_list<ValueType>&& iterable, int64_t) -> enumerate<decltype(iterable)>;
#endif
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // EnumerateState - This represents the state of the enumerate generator
    ////////////////////////////////////////////////////////////////////////////////
    template<class UnderlyingIterator>
    struct EnumerateState {
        int64_t idx_;
        UnderlyingIterator iter_;

        // Enables structured bindings
        template <std::size_t N>
        constexpr decltype(auto) get() const {
            if      constexpr (N == 0) return idx_;
            else if constexpr (N == 1) return *iter_;
        }
    };
}

// Need to do this since array and tuple define tuple_element and tuple_size as a class and a struct
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

////////////////////////////////////////////////////////////////////////////////
// tuple_element and tuple_size specializations for EnumerateState's structured bindings
////////////////////////////////////////////////////////////////////////////////
namespace std {
    template<std::size_t N, class UnderlyingIterator>
    struct tuple_element<N, EnumerateState<UnderlyingIterator>> {
        using type = decltype(std::declval<EnumerateState<UnderlyingIterator>>().template get<N>());
    };

    template<class UnderlyingIterator>
    struct tuple_size<EnumerateState<UnderlyingIterator>> : std::integral_constant<std::size_t, 2> {
        // Empty
    };
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
