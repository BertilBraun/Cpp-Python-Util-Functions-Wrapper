#pragma once

#include "generator_iterator.h"

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
            , modded_end_(end * comparison_mod_)
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
        return Range{{0, end, 1}};
    }

    constexpr Range range(int64_t begin, int64_t end) {
        return Range{{begin, end, 1}};
    }

    constexpr Range range(int64_t begin, int64_t end, int64_t step) {
        return Range{{begin, end, step}};
    }
}
