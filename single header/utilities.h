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
        State state_{ starting_idx_, std::begin(iterable_) };

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
        constexpr Iterator begin() { return Iterator{ *this }; }
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
    enumerate(Iterable&& iterable)->enumerate<decltype(iterable)>;

    template<class Iterable>
    enumerate(Iterable&& iterable, int64_t)->enumerate<decltype(iterable)>;

    // Clang and MSVC do not seem to be able to support this for various reasons at this time
#ifdef __GNUC__
#ifndef __clang__
    template<class ValueType> enumerate(std::initializer_list<ValueType>&& iterable)->enumerate<decltype(iterable)>;
    template<class ValueType> enumerate(std::initializer_list<ValueType>&& iterable, int64_t)->enumerate<decltype(iterable)>;
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


namespace utilities::intern {
    ////////////////////////////////////////////////////////////////////////////////
    // Generator - This class encapsulates the common code of generator style
    // classes and enforces a common structure / behavior.
    ////////////////////////////////////////////////////////////////////////////////
    template<class Impl>
    class Generator : public Impl {
    private:
        //------------------------------------------------------------------------------
        // Types
        //------------------------------------------------------------------------------
        using Iterator = GeneratorIterator<Generator>;

    public:
        //------------------------------------------------------------------------------
        // begin/end - standard iterator access and enable use in ranged-for loops
        //------------------------------------------------------------------------------
        constexpr Iterator begin() { return Iterator(*this); }
        constexpr GeneratorEnd end() { return {}; }

        //------------------------------------------------------------------------------
        // operators
        // operator* - This should return the current state of the generator
        // operator++ - This should advance the generator. Note only the prefix++ is provided.
        //              Due to the generator having one state and not being copyable
        //              a postfix++ doesn't make sense and would only provide behavior
        //              that is inconsistent with a regular postfix++ operator.
        // operator bool - This should return true while the generator can still be advanced.
        //                 Once the generator should not be advanced anymore, this should return false
        //------------------------------------------------------------------------------
        constexpr decltype(auto) operator*() { return Impl::operator*(); }
        constexpr Generator& operator++() { Impl::operator++(); return *this; }
        constexpr explicit operator bool() const { return Impl::operator bool(); }

    public:
        //------------------------------------------------------------------------------
        // Implicit Constructors/assignment - All deleted
        // Due to the nature of a generator, moving or copying doesn't make sense
        //------------------------------------------------------------------------------
        constexpr Generator(const Generator&) = delete;
        constexpr Generator(Generator&&) = delete;
        constexpr Generator& operator=(const Generator&) = delete;
        constexpr Generator& operator=(Generator&&) = delete;
    };
}

namespace {
    ////////////////////////////////////////////////////////////////////////////////
    // RangeImpl - This class is the implementation for the range generator
    ////////////////////////////////////////////////////////////////////////////////
    class RangeImpl {
    public:
        //------------------------------------------------------------------------------
        // Constructor - Must be initialized with a begin, end, and step
        //------------------------------------------------------------------------------
        constexpr RangeImpl(int64_t begin, int64_t end, int64_t step)
            : val_(begin)
            , step_(step)
            , comparison_mod_(step < 0 ? -1 : 1)
            , modded_end_(end* comparison_mod_)
        {
            // Nothing
        }

    public:
        //------------------------------------------------------------------------------
        // operators - Provides the state, advances the generator, and determines completion
        //------------------------------------------------------------------------------
        constexpr int64_t operator*() { return val_; }
        constexpr RangeImpl& operator++() { val_ += step_; return *this; }
        constexpr explicit operator bool() const { return (val_ * comparison_mod_) < modded_end_; }

    private:
        //------------------------------------------------------------------------------
        // Member Variables
        //------------------------------------------------------------------------------
        int64_t val_;
        const int64_t step_;
        const int64_t comparison_mod_;
        const int64_t modded_end_;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Define the Range generator
    ////////////////////////////////////////////////////////////////////////////////
    using Range = utilities::intern::Generator<RangeImpl>;

    ////////////////////////////////////////////////////////////////////////////////
    // range function shortcuts - begin is inclusive, end is exclusive - [begin, end)
    //      range(end) - begin is 0 and step is 1
    //      range(begin, end) - step is 1
    //      range(begin, end, step) - begin is 0 and step is 1
    ////////////////////////////////////////////////////////////////////////////////
    constexpr Range range(int64_t end) {
        return Range{ {0, end, 1} };
    }

    constexpr Range range(int64_t begin, int64_t end) {
        return Range{ {begin, end, 1} };
    }

    constexpr Range range(int64_t begin, int64_t end, int64_t step) {
        return Range{ {begin, end, step} };
    }
}

namespace {
    ////////////////////////////////////////////////////////////////////////////////
    // Forward Declarations
    ////////////////////////////////////////////////////////////////////////////////
    template<size_t IDX, class... Iterables>
    struct ZipStorage;

    template<size_t IDX, class... Iterables>
    struct ZipState;

    ////////////////////////////////////////////////////////////////////////////////
    // zip - This class behaves like a Generator, but does not inherit from
    // Generator because of a clang bug related to rvalue reference lifetime
    // extensions in base classes.  So the Generator behavior is emulated instead.
    ////////////////////////////////////////////////////////////////////////////////
    template<class... Iterables>
    class zip {
    public:
        //------------------------------------------------------------------------------
        // Initialization variables - Can only be set via aggregate initialization
        //------------------------------------------------------------------------------
        using Storage = ZipStorage<0, Iterables...>;
        Storage storage_;

    public:
        //------------------------------------------------------------------------------
        // State of the generator - It's public to allow for aggregate initialization of
        // the above variables, but this should be treated as private.
        //------------------------------------------------------------------------------
        using State = ZipState<0, Iterables...>;
        State state_{ storage_ };

    public:
        //------------------------------------------------------------------------------
        // Implicit Constructors - All Deleted
        //------------------------------------------------------------------------------
        constexpr zip() = delete;
        constexpr zip(const zip&) = delete;
        constexpr zip(zip&&) = delete;
        constexpr zip& operator=(const zip&) = delete;
        constexpr zip& operator=(zip&&) = delete;

    public:
        //------------------------------------------------------------------------------
        // Begin and End - Enables usage in ranged-for
        //------------------------------------------------------------------------------
        using Iterator = utilities::intern::GeneratorIterator<zip>;
        constexpr Iterator begin() { return Iterator{ *this }; }
        constexpr utilities::intern::GeneratorEnd end() { return {}; }

    public:
        //------------------------------------------------------------------------------
        // operators - Emulating the generator class's operators
        //------------------------------------------------------------------------------
        constexpr State& operator*() { return state_; }
        constexpr zip& operator++() { ++state_; return *this; }
        constexpr explicit operator bool() const { return state_.can_advance(storage_); }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Deduction guides
    //      zip{some_iterable, more_iterables...}
    ////////////////////////////////////////////////////////////////////////////////

    template<class RequiredIterable, class... OptionalIterables>
    zip(RequiredIterable&& required_iterable, OptionalIterables&&... optional_iterables)
        ->zip<decltype(required_iterable), decltype(optional_iterables)...>;

    ////////////////////////////////////////////////////////////////////////////////
    // ZipStorage - Holds references to the passed in iterators
    ////////////////////////////////////////////////////////////////////////////////
    template<size_t IDX, class CurrentIterable>
    struct ZipStorage<IDX, CurrentIterable> {
        CurrentIterable iterable;

        constexpr auto begin() { return std::begin(iterable); }
        constexpr auto end() const { return std::end(iterable); }
    };

    template<size_t IDX, class CurrentIterable, class... RemainingIterables>
    struct ZipStorage<IDX, CurrentIterable, RemainingIterables...> {
        CurrentIterable iterable;

        using NextStorage = ZipStorage<IDX + 1, RemainingIterables...>;
        NextStorage next_storage;

        constexpr auto begin() { return std::begin(iterable); }
        constexpr auto end() const { return std::end(iterable); }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // ZipState - This represents the state of the zip generator
    ////////////////////////////////////////////////////////////////////////////////
    template<size_t IDX, class CurrentIterable>
    struct ZipState<IDX, CurrentIterable> {
        using Storage = ZipStorage<IDX, CurrentIterable>;

        // Construct from a ZipStorage matching this depth
        explicit constexpr ZipState(Storage& storage)
            : iterator(storage.begin())
        {
            // Nothing
        }

        // Increment this iterator
        constexpr void operator++() { ++iterator; }
        constexpr bool can_advance(const Storage& storage) const {
            return iterator != storage.end();
        }

        // Enables structured bindings
        template <std::size_t N>
        constexpr decltype(auto) get() const {
            if constexpr (N == IDX) { return *iterator; }
        }

        // Member Variables
        using Iterator = decltype(std::declval<ZipStorage<IDX, CurrentIterable>>().begin());
        Iterator iterator;
    };

    template<size_t IDX, class CurrentIterable, class... RemainingIterables>
    struct ZipState<IDX, CurrentIterable, RemainingIterables...> {
        using Storage = ZipStorage<IDX, CurrentIterable, RemainingIterables...>;

        // Construct from a ZipStorage matching this depth
        explicit constexpr ZipState(Storage& storage)
            : iterator(storage.begin())
            , next_state(storage.next_storage)
        {
            // Nothing
        }

        // Increment this iterator and all remaining iterators
        constexpr void operator++() { ++iterator; ++next_state; }
        constexpr bool can_advance(const Storage& storage) const {
            return iterator != storage.end() && next_state.can_advance(storage.next_storage);
        }

        // Enables structured bindings
        template <std::size_t N>
        constexpr decltype(auto) get() const {
            if constexpr (N == IDX) { return *iterator; }
            else { return next_state.template get<N>(); }
        }

        // Member Variables
        using Iterator = decltype(std::declval<ZipStorage<IDX, CurrentIterable>>().begin());
        Iterator iterator;

        using NextState = ZipState<IDX + 1, RemainingIterables...>;
        NextState next_state;
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
    template<std::size_t N, class... Iterables>
    struct tuple_element<N, ZipState<0, Iterables...>> {
        using type = decltype(std::declval<ZipState<0, Iterables...>>().template get<N>());
    };

    template<class... Iterables>
    struct tuple_size<ZipState<0, Iterables...>> : std::integral_constant<std::size_t, sizeof...(Iterables)> {
        // Empty
    };
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
