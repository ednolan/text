#include <boost/text/algorithm.hpp>

#include <boost/text/string_view.hpp>
#include <boost/text/string.hpp>
#include <boost/text/unencoded_rope.hpp>
#include <boost/text/unencoded_rope_view.hpp>
#include <boost/text/text.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/rope_view.hpp>

#include <boost/range/iterator_range_core.hpp>

#include <boost/mpl/assert.hpp> // TODO

#include <array>
#include <list>
#include <string>
#include <vector>


struct a_t
{
    using iterator = std::array<char, 4>::const_iterator;
    std::array<char, 4> chars_;
};

std::array<char, 4>::const_iterator begin(a_t const & a)
{
    return a.chars_.begin();
}
std::array<char, 4>::const_iterator end(a_t const & a)
{
    return a.chars_.end();
}

struct b_t
{
    using iterator = std::array<char, 4>::const_iterator;

    std::array<char, 4>::const_iterator begin() const { return chars_.begin(); }
    std::array<char, 4>::const_iterator end() const { return chars_.end(); }

    std::array<char, 4> chars_;
};


using namespace boost;

static_assert(text::detail::is_char_range<text::string_view>::value, "");
static_assert(text::detail::is_char_range<text::string_view const>::value, "");
static_assert(text::detail::is_char_range<text::string>::value, "");
static_assert(text::detail::is_char_range<text::string const>::value, "");
static_assert(text::detail::is_char_range<text::unencoded_rope>::value, "");
static_assert(text::detail::is_char_range<text::unencoded_rope_view>::value, "");
static_assert(text::detail::is_char_range<std::string>::value, "");
static_assert(text::detail::is_char_range<std::string const>::value, "");
static_assert(text::detail::is_char_range<std::vector<char>>::value, "");
static_assert(text::detail::is_char_range<std::vector<char> const>::value, "");
static_assert(text::detail::is_char_range<std::array<char, 5>>::value, "");
static_assert(text::detail::is_char_range<std::array<char, 5> const>::value, "");
static_assert(text::detail::is_char_range<iterator_range<char *>>::value, "");
static_assert(text::detail::is_char_range<iterator_range<char const *>>::value, "");
static_assert(
    text::detail::is_char_range<iterator_range<std::vector<char>::iterator>>::value,
    "");
static_assert(
    text::detail::is_char_range<
        iterator_range<std::vector<char>::iterator const>>::value,
    "");

static_assert(text::detail::is_char_range<a_t>::value, "");
static_assert(text::detail::is_char_range<a_t const>::value, "");
static_assert(text::detail::is_char_range<b_t>::value, "");
static_assert(text::detail::is_char_range<b_t const>::value, "");

static_assert(!text::detail::is_char_range<std::list<char>>::value, "");
static_assert(!text::detail::is_char_range<std::vector<wchar_t>>::value, "");
static_assert(!text::detail::is_char_range<std::vector<int>>::value, "");
static_assert(!text::detail::is_char_range<std::array<float, 5>>::value, "");
static_assert(!text::detail::is_char_range<char>::value, "");
static_assert(!text::detail::is_char_range<int>::value, "");
static_assert(!text::detail::is_char_range<iterator_range<wchar_t *>>::value, "");
static_assert(
    !text::detail::is_char_range<iterator_range<std::vector<int>::iterator>>::value,
    "");
static_assert(!text::detail::is_char_range<wchar_t[5]>::value, "");
static_assert(!text::detail::is_char_range<int[5]>::value, "");

// This may look wrong, but we don't care, because this decays to a char *
// anyway!
static_assert(!text::detail::is_char_range<char[5]>::value, "");

static_assert(!text::detail::is_char_range<text::text>::value, "");
static_assert(!text::detail::is_char_range<text::text_view>::value, "");
static_assert(!text::detail::is_char_range<text::rope>::value, "");
static_assert(!text::detail::is_char_range<text::rope_view>::value, "");


static_assert(!text::detail::is_contig_char_range<text::unencoded_rope>::value, "");
static_assert(!text::detail::is_contig_char_range<text::unencoded_rope_view>::value, "");


static_assert(text::detail::is_grapheme_char_range<text::text>::value, "");
static_assert(text::detail::is_grapheme_char_range<text::text_view>::value, "");
static_assert(text::detail::is_grapheme_char_range<text::rope>::value, "");
static_assert(text::detail::is_grapheme_char_range<text::rope_view>::value, "");

static_assert(text::detail::is_contig_grapheme_char_range<text::text>::value, "");
static_assert(text::detail::is_contig_grapheme_char_range<text::text_view>::value, "");
static_assert(!text::detail::is_contig_grapheme_char_range<text::rope>::value, "");
static_assert(!text::detail::is_contig_grapheme_char_range<text::rope_view>::value, "");
