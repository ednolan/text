// Copyright (C) 2023 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_ITERATOR_FWD_HPP
#define BOOST_TEXT_TRANSCODE_ITERATOR_FWD_HPP

#include <boost/text/concepts.hpp>


namespace boost { namespace text {

    namespace detail {
        template<
            typename RepackedIterator,
            typename I,
            typename S,
            typename Then>
        struct bidi_repacker;
    }
}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    namespace dtl {
        template<format Format>
        constexpr auto format_to_type();

        template<format Format>
        using format_to_type_t = decltype(format_to_type<Format>());
    }

    template<
        format FromFormat,
        format ToFormat,
        std::input_iterator I,
        std::sentinel_for<I> S = I>
        requires std::convertible_to<std::iter_value_t<I>, dtl::format_to_type_t<FromFormat>>
    class utf_iterator;

    template<
        utf8_iter I,
        std::sentinel_for<I> S = I>
    using utf_8_to_16_iterator =
        utf_iterator<format::utf8, format::utf16, I, S>;
    template<
        utf16_iter I,
        std::sentinel_for<I> S = I>
    using utf_16_to_8_iterator =
        utf_iterator<format::utf16, format::utf8, I, S>;


    template<
        utf8_iter I,
        std::sentinel_for<I> S = I>
    using utf_8_to_32_iterator =
        utf_iterator<format::utf8, format::utf32, I, S>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I>
    using utf_32_to_8_iterator =
        utf_iterator<format::utf32, format::utf8, I, S>;


    template<
        utf16_iter I,
        std::sentinel_for<I> S = I>
    using utf_16_to_32_iterator =
        utf_iterator<format::utf16, format::utf32, I, S>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I>
    using utf_32_to_16_iterator =
        utf_iterator<format::utf32, format::utf16, I, S>;

}}}

#endif
