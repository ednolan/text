// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_GRAPHEME_ITERATOR_HPP
#define BOOST_TEXT_GRAPHEME_ITERATOR_HPP

#include <boost/text/config.hpp>
#include <boost/text/grapheme.hpp>
#include <boost/text/grapheme_break.hpp>

#include <boost/assert.hpp>
#include <boost/stl_interfaces/iterator_interface.hpp>

#include <iterator>
#include <type_traits>
#include <stdexcept>


namespace boost { namespace text {

    // TODO: forward and input feriendliness
    /** A bidirectional filtering iterator that iterates over the extended
        grapheme clusters in a sequence of code points. */
    template<code_point_iter I, std::sentinel_for<I> S = I>
    class grapheme_iterator : public stl_interfaces::iterator_interface<
                                  grapheme_iterator<I, S>,
                                  decltype(detail::bidirectional_at_most<I>()),
                                  grapheme_ref<I>,
                                  grapheme_ref<I>>
    {
    public:
        constexpr grapheme_iterator() = default;

        constexpr grapheme_iterator(
            I first, I it, S last) :
            first_(text::unpack_iterator_and_sentinel(first, last).first),
            grapheme_first_(text::unpack_iterator_and_sentinel(it, last).first),
            grapheme_last_(text::unpack_iterator_and_sentinel(
                               boost::text::next_grapheme_break(it, last), last)
                               .first),
            last_(text::unpack_iterator_and_sentinel(first, last).last)
        {}

        template<class I2, class S2>
            requires std::convertible_to<I2, I> && std::convertible_to<S2, S>
        constexpr grapheme_iterator(grapheme_iterator<I2, S2> const & other) :
            first_(other.first_),
            grapheme_first_(other.grapheme_first_),
            grapheme_last_(other.grapheme_last_),
            last_(other.last_)
        {}

        constexpr I begin() const requires std::bidirectional_iterator<I> { return seq_begin(); }
        constexpr S end() const { return seq_end(); }

        constexpr I base() const { return gr_begin(); }

        constexpr grapheme_ref<I> operator*() const
        {
            return grapheme_ref<I>(gr_begin(), gr_end());
        }

        constexpr grapheme_iterator & operator++()
        {
            I next_break =
                boost::text::next_grapheme_break(gr_end(), seq_end());
            grapheme_first_ = grapheme_last_;
            grapheme_last_ =
                text::unpack_iterator_and_sentinel(next_break, seq_end()).first;
            return *this;
        }

        constexpr grapheme_iterator & operator--()
        {
            I prev_break = boost::text::prev_grapheme_break(
                seq_begin(), std::ranges::prev(gr_begin()), seq_end());
            grapheme_last_ = grapheme_first_;
            grapheme_first_ =
                text::unpack_iterator_and_sentinel(prev_break, seq_end()).first;
            return *this;
        }

        friend constexpr bool
        operator==(grapheme_iterator lhs, grapheme_iterator rhs)
        {
            return lhs.base() == rhs.base();
        }

        friend constexpr bool
        operator==(grapheme_iterator lhs, S rhs) requires(!std::same_as<I, S>)
        {
            return lhs.base() == rhs;
        }

    private:
        using cu_iterator =
            decltype(text::unpack_iterator_and_sentinel(
                         std::declval<I>(), std::declval<S>())
                         .first);
        using cu_sentinel =
            decltype(text::unpack_iterator_and_sentinel(
                         std::declval<I>(), std::declval<S>())
                         .last);

        template<typename ResultType, typename I1, typename I2, typename S2>
        static auto make_iter(I1 first, I2 it, S2 last)
        {
            if constexpr (requires { ResultType(first, it, last); }) {
                return ResultType{first, it, last};
            } else {
                return it;
            }
        }

        constexpr I seq_begin() const
        {
            return make_iter<I>(first_, first_, last_);
        }
        constexpr I gr_begin() const
        {
            return make_iter<I>(first_, grapheme_first_, last_);
        }
        constexpr I gr_end() const
        {
            return make_iter<I>(first_, grapheme_last_, last_);
        }
        constexpr S seq_end() const
        {
            return make_iter<S>(first_, last_, last_);
        }

        cu_iterator first_;
        cu_iterator grapheme_first_;
        cu_iterator grapheme_last_;
        [[no_unique_address]] cu_sentinel last_;

        template<code_point_iter I2, std::sentinel_for<I2> S2>
        friend struct grapheme_iterator;
    };

}}

#endif
