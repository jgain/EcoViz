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
 * Define uts::vector as either std::vector or __gnu_debug::vector depending on
 * UTS_DEBUG_CONTAINERS.
 */

#ifndef UTS_COMMON_DEBUG_VECTOR
#define UTS_COMMON_DEBUG_VECTOR

#include <vector>
#if defined(__GLIBCXX__) && defined(UTS_DEBUG_CONTAINERS)

#include <debug/vector>

namespace uts
{
    template<typename T, typename Alloc = std::allocator<T> >
    using vector = __gnu_debug::vector<T, Alloc>;
}

#else

namespace uts
{
    template<typename T, typename Alloc = std::allocator<T> >
    using vector = std::vector<T, Alloc>;
}

#endif

#endif /* !UTS_COMMON_DEBUG_VECTOR */
