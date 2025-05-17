/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  B. Merry
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/
/**
 * Define uts::string as either std::string or __gnu_debug::string depending on
 * UTS_DEBUG_CONTAINERS.
 */

#ifndef UTS_COMMON_DEBUG_STRING
#define UTS_COMMON_DEBUG_STRING

#include <string>
#if defined(__GLIBCXX__) && defined(UTS_DEBUG_CONTAINERS)

#include <debug/string>

namespace uts
{
    template<
        typename CharT,
        typename Traits = std::char_traits<CharT>,
        typename Alloc = std::allocator<CharT> >
    using basic_string = __gnu_debug::basic_string<CharT, Traits, Alloc>;
}

namespace std
{
    template<>
    struct hash<__gnu_debug::string>
    {
        size_t operator()(const __gnu_debug::string &s) const noexcept
        {
            return hash<std::string>()(s);
        }
    };
}

#else

namespace uts
{
    template<
        typename CharT,
        typename Traits = std::char_traits<CharT>,
        typename Alloc = std::allocator<CharT> >
    using basic_string = std::basic_string<CharT, Traits, Alloc>;
}

#endif

namespace uts
{
    typedef uts::basic_string<char> string;
    typedef basic_string<wchar_t> wstring;
    typedef basic_string<char16_t> u16string;
    typedef basic_string<char32_t> u32string;
}

#endif /* !UTS_COMMON_DEBUG_STRING */
