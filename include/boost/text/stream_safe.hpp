// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_STREAM_SAFE_HPP
#define BOOST_TEXT_STREAM_SAFE_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/dangling.hpp>
#include <boost/text/detail/normalization_data.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    namespace detail {
        enum : std::size_t { stream_safe_max_nonstarters = 9 };

        template<typename CPIter, typename Sentinel>
        CPIter next_stream_safe_cp(
            CPIter first, Sentinel last, std::size_t & nonstarters)
        {
            for (; first != last; ++first) {
                auto const cp = *first;
                if (detail::ccc(cp) == 0)
                    nonstarters = 0;
                else
                    ++nonstarters;
                if (nonstarters < stream_safe_max_nonstarters)
                    break;
            }
            return first;
        }

        template<typename Iter, typename Sentinel, typename Repack>
        auto
        stream_safe_out(tagged_range<utf8_tag, Iter, Sentinel, Repack> unpacked)
        {
            return boost::text::utf_32_to_8_out(unpacked.f_);
        }
        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            tagged_range<utf16_tag, Iter, Sentinel, Repack> unpacked)
        {
            return boost::text::utf_32_to_16_out(unpacked.f_);
        }
        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            tagged_range<utf32_tag, Iter, Sentinel, Repack> unpacked)
        {
            return unpacked.f_;
        }

        template<
            typename OutIter,
            typename Iter,
            typename Sentinel,
            typename Repack>
        auto stream_safe_result(
            tagged_range<utf8_tag, Iter, Sentinel, Repack> unpacked,
            OutIter out)
        {
            return boost::text::utf32_iterator(
                unpacked.f_, out.base(), unpacked.l_);
        }
        template<
            typename OutIter,
            typename Iter,
            typename Sentinel,
            typename Repack>
        auto stream_safe_result(
            tagged_range<utf16_tag, Iter, Sentinel, Repack> unpacked,
            OutIter out)
        {
            return boost::text::utf32_iterator(
                unpacked.f_, out.base(), unpacked.l_);
        }
        template<typename OutIter>
        auto stream_safe_result(utf32_tag, OutIter out)
        {
            return out;
        }

        template<typename I, typename S, typename O>
        O stream_safe_copy_impl(I first, S last, O out)
        {
            if (first == last)
                return out;
            std::size_t nonstarters = detail::ccc(*first) ? 1 : 0;
            do {
                *out = *first;
                ++out;
                ++first;
                first = detail::next_stream_safe_cp(first, last, nonstarters);
            } while (first != last);
            return out;
        }

        template<typename I, typename S>
        auto stream_safe_impl(I first, S last)
        {
            auto const unpacked =
                detail::unpack_iterator_and_sentinel(first, last);
            auto out = detail::stream_safe_out(unpacked);
            out = detail::stream_safe_copy_impl(first, last, out);
            return detail::stream_safe_result(unpacked, out);
        }

        template<typename I, typename S>
        bool is_stream_safe_impl(I first, S last)
        {
            std::size_t nonstarters = 0;
            for (; first != last; ++first) {
                auto const cp = *first;
                if (detail::ccc(cp) == 0)
                    nonstarters = 0;
                else
                    ++nonstarters;
                if (stream_safe_max_nonstarters <= nonstarters)
                    return false;
            }
            return true;
        }
    }

    /** An iterator that filters out all the non-stream-safe code points from
        a sequence of code points.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct stream_safe_iterator : stl_interfaces::iterator_interface<
                                      stream_safe_iterator<I, S>,
                                      std::bidirectional_iterator_tag,
                                      uint32_t,
                                      uint32_t>
    {
        using iterator = I;
        using sentinel = S;

        stream_safe_iterator() noexcept : first_(), it_(), last_() {}
        stream_safe_iterator(iterator first, sentinel last) noexcept :
            first_(first),
            it_(first),
            last_(last),
            nonstarters_(it_ != last_ && detail::ccc(*it_) ? 1 : 0)
        {}

        uint32_t operator*() const noexcept
        {
            BOOST_ASSERT(it_ != last_);
            return *it_;
        }

        constexpr I base() const noexcept { return it_; }

        stream_safe_iterator & operator++() noexcept
        {
            BOOST_ASSERT(it_ != last_);
            ++it_;
            it_ = detail::next_stream_safe_cp(it_, last_, nonstarters_);
            return *this;
        }

        stream_safe_iterator & operator--() noexcept
        {
            BOOST_ASSERT(it_ != first_);
            if (0 < nonstarters_) {
                --it_;
            } else {
                auto const it = boost::text::find_if_backward(
                    first_, it_, [](auto cp) { return detail::ccc(cp) == 0; });
                auto const from = it == it_ ? first_ : std::next(it);
                std::size_t const nonstarters = std::distance(from, it_);
                nonstarters_ = (std::min)(
                    nonstarters, detail::stream_safe_max_nonstarters - 1);
                it_ = std::next(it == it_ ? first_ : it, nonstarters_);
            }
            return *this;
        }

        friend bool
        operator==(stream_safe_iterator lhs, stream_safe_iterator rhs) noexcept
        {
            return lhs.base() == rhs.base();
        }

        using base_type = stl_interfaces::iterator_interface<
            stream_safe_iterator<I, S>,
            std::bidirectional_iterator_tag,
            uint32_t,
            uint32_t>;
        using base_type::operator++;
        using base_type::operator--;

    private:
        iterator first_;
        iterator it_;
        [[no_unique_address]] sentinel last_;
        std::size_t nonstarters_ = 0;
    };

    template<typename I, typename S>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator==(stream_safe_iterator<I, S> const & lhs, S rhs)
        -> decltype(lhs.base() == rhs)
    {
        return lhs.base() == rhs.base();
    }
    template<typename I, typename S>
    auto operator==(S lhs, stream_safe_iterator<I, S> const & rhs)
        -> decltype(rhs.base() == lhs)
    {
        return rhs.base() == lhs;
    }
    template<typename I, typename S>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator!=(stream_safe_iterator<I, S> const & lhs, S rhs)
        -> decltype(lhs.base() != rhs)
    {
        return lhs.base() != rhs.base();
    }
    template<typename I, typename S>
    auto operator!=(S lhs, stream_safe_iterator<I, S> const & rhs)
        -> decltype(rhs.base() != lhs)
    {
        return rhs.base() != lhs;
    }

    namespace detail {
        template<typename T>
        struct is_stream_safe_iter : std::false_type
        {};
        template<typename T, typename U>
        struct is_stream_safe_iter<stream_safe_iterator<T, U>> : std::true_type
        {};
    }

    /** A view that adapts a non-stream safe sequence of code points to a
        stream-safe sequence of code points.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    // clang-format off
#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS
    template<typename I, std::sentinel_for<I> S = I>
        requires detail::is_stream_safe_iter<I>::value
#else
    template<typename I, typename S = I>
#endif
    struct stream_safe_view
        // clang-format on
        : stl_interfaces::view_interface<stream_safe_view<I, S>>
    {
        using iterator = I;
        using sentinel = S;

        constexpr stream_safe_view() noexcept {}
        constexpr stream_safe_view(iterator first, sentinel last) noexcept :
            first_(first), last_(last)
        {}

        constexpr iterator begin() const noexcept { return first_; }
        constexpr sentinel end() const noexcept { return last_; }

        friend constexpr bool
        operator==(stream_safe_view lhs, stream_safe_view rhs)
        {
            return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
        }

    private:
        iterator first_;
        [[no_unique_address]] sentinel last_;
    };

    namespace detail {
        template<typename I, typename S>
        constexpr auto as_stream_safe_impl(I first, S last) noexcept
        {
            using iterator = stream_safe_iterator<I, S>;
            return stream_safe_view<iterator, S>{iterator{first, last}, last};
        }

        template<typename I>
        constexpr auto as_stream_safe_impl(I first, I last) noexcept
        {
            using iterator = stream_safe_iterator<I>;
            return stream_safe_view<iterator>{
                iterator{first, last}, iterator{last, last}};
        }
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

    /** Writes sequence `[first, last)` to `out`, ensuring Stream-Safe Text
        Format.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPIter, typename Sentinel, typename OutIter>
    constexpr auto stream_safe_copy(
        CPIter first,
        Sentinel last,
        OutIter out) noexcept->detail::cp_iter_ret_t<OutIter, CPIter>
    {
        return detail::stream_safe_copy_impl(first, last, out);
    }

    /** Writes sequence `r` to `out`, ensuring Stream-Safe Text Format.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPRange, typename OutIter>
    constexpr auto stream_safe_copy(CPRange && r, OutIter out) noexcept
        ->decltype(boost::text::v1::stream_safe_copy(
            detail::begin(r), detail::end(r), out))
    {
        return boost::text::v1::stream_safe_copy(
            detail::begin(r), detail::end(r), out);
    }

    /** Copies the stream-safe portion of `[first, last)` to the beginning
       of
        `[first, last)`, and returns an iterator to the end of the copied
        range.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPIter, typename Sentinel>
    constexpr auto stream_safe(
        CPIter first,
        Sentinel last) noexcept->detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::stream_safe_impl(first, last);
    }

    /** Copies the stream-safe portion of `r` to the beginning of `r`, and
        returns an iterator to the end of the copied range.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPRange>
    constexpr auto stream_safe(CPRange && r) noexcept
        ->decltype(boost::text::v1::stream_safe(detail::begin(r), detail::end(r)))
    {
        return boost::text::v1::stream_safe(detail::begin(r), detail::end(r));
    }

    /** Returns true iff `[first, last)` is in stream-safe format.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPIter, typename Sentinel>
    constexpr auto is_stream_safe(
        CPIter first,
        Sentinel last) noexcept->detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::is_stream_safe_impl(first, last);
    }

    /** Returns true iff `r` is in stream-safe format.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPRange>
    constexpr auto is_stream_safe(CPRange && r) noexcept
        ->decltype(boost::text::v1::is_stream_safe(
            detail::begin(r), detail::end(r)))
    {
        return boost::text::v1::is_stream_safe(detail::begin(r), detail::end(r));
    }

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns a `stream_safe_view` of the range `[first, last)`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPIter, typename Sentinel>
    constexpr detail::unspecified as_stream_safe(
        CPIter first, Sentinel last) noexcept;

    /** Returns a `stream_safe_view` of the range `[first, last)`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPIter>
    constexpr detail::unspecified as_stream_safe(
        CPIter first, CPIter last) noexcept;

    /** Returns a `stream_safe_view` of the range `r`.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept.  The result is returned as a
        `borrowed_view_t` in C++20 and later.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPRange>
    constexpr detail::unspecified as_stream_safe(CPRange && r) noexcept;

#endif

    namespace dtl {
        struct as_stream_safe_impl : range_adaptor_closure<as_stream_safe_impl>
        {
            template<typename CPIter, typename Sentinel>
            constexpr auto
            operator()(CPIter first, Sentinel last) const noexcept
                -> detail::cp_iter_ret_t<
                    stream_safe_view<
                        stream_safe_iterator<CPIter, Sentinel>,
                        Sentinel>,
                    CPIter>
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<typename CPIter>
            constexpr auto
            operator()(CPIter first, CPIter last) const noexcept
                -> detail::cp_iter_ret_t<
                    stream_safe_view<stream_safe_iterator<CPIter>>,
                    CPIter>
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<typename CPRange>
            constexpr auto operator()(CPRange && r) const noexcept
                -> decltype((*this)(detail::begin(r), detail::end(r)))
            {
                return (*this)(detail::begin(r), detail::end(r));
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_stream_safe_impl as_stream_safe;
#else
    namespace {
        constexpr dtl::as_stream_safe_impl as_stream_safe;
    }
#endif

}}}

#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::output_iterator<uint32_t> O>
    constexpr O stream_safe_copy(I first, S last, O out) noexcept
    {
        return detail::stream_safe_copy_impl(first, last, out);
    }

    template<code_point_range R, std::output_iterator<uint32_t> O>
    constexpr O stream_safe_copy(R && r, O out) noexcept
    {
        return boost::text::stream_safe_copy(
            std::ranges::begin(r), std::ranges::end(r), out);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr auto stream_safe(I first, S last) noexcept
    {
        return detail::stream_safe_impl(first, last);
    }

    template<code_point_range R>
    constexpr std::ranges::borrowed_iterator_t<R> stream_safe(
        R && r) noexcept
    {
        return boost::text::stream_safe(
            std::ranges::begin(r), std::ranges::end(r));
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr bool is_stream_safe(I first, S last) noexcept
    {
        return detail::is_stream_safe_impl(first, last);
    }

    template<code_point_range R>
    constexpr bool is_stream_safe(R && r) noexcept
    {
        return boost::text::is_stream_safe(
            std::ranges::begin(r), std::ranges::end(r));
    }

    namespace dtl {
        struct as_stream_safe_impl : range_adaptor_closure<as_stream_safe_impl>
        {
            template<code_point_iter I, std::sentinel_for<I> S>
            constexpr stream_safe_view<stream_safe_iterator<I, S>, S>
            operator()(I first, S last) const noexcept
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<code_point_iter I>
            constexpr stream_safe_view<stream_safe_iterator<I>>
            operator()(I first, I last) const noexcept
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<code_point_range R>
            constexpr auto operator()(R && r) const noexcept
            {
                if constexpr (std::ranges::borrowed_range<R>)
                    return (*this)(std::ranges::begin(r), std::ranges::end(r));
                else
                    return std::ranges::dangling{};
            }
        };
    }

    inline constexpr dtl::as_stream_safe_impl as_stream_safe;

}}}

namespace std::ranges {
    template<typename I, std::sentinel_for<I> S>
    // clang-format off
    requires boost::text::detail::is_stream_safe_iter<I>::value
    inline constexpr bool
        // clang-format on
        enable_borrowed_range<boost::text::stream_safe_view<I, S>> = true;
}

#endif

#endif
