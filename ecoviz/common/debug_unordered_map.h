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
 * @file
 *
 * Define uts::unordered_map as either std::unordered_map or
 * __gnu_debug::unordered_map depending on UTS_DEBUG_CONTAINERS.
 */

#ifndef UTS_COMMON_DEBUG_UNORDERED_MAP
#define UTS_COMMON_DEBUG_UNORDERED_MAP

#include <unordered_map>
#if defined(__GLIBCXX__) && defined(UTS_DEBUG_CONTAINERS)

#include <debug/unordered_map>

namespace uts
{
    template<
        typename Key,
        typename T,
        typename Hash = std::hash<Key>,
        typename KeyEqual = std::equal_to<Key>,
        typename Allocator = std::allocator<std::pair<const Key, T> > >
    using unordered_map = __gnu_debug::unordered_map<Key, T, Hash, KeyEqual, Allocator>;
}

#else

namespace uts
{
    template<
        typename Key,
        typename T,
        typename Hash = std::hash<Key>,
        typename KeyEqual = std::equal_to<Key>,
        typename Allocator = std::allocator<std::pair<const Key, T> > >
    using unordered_map = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;
}

#endif

#endif /* !UTS_COMMON_DEBUG_UNORDERED_MAP */
