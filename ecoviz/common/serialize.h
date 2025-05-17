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
 * Serialization helpers.
 */

#include <array>
#include <cstddef>
#include <boost/serialization/array.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>
#include "debug_vector.h"
#include "debug_list.h"

namespace boost
{
namespace serialization
{
/*
template<typename Archive, typename T, std::size_t N>
void serialize(Archive &ar, std::array<T, N> &a, const unsigned int)
{
    ar & make_array(a.data(), N);
}*/

#if defined(__GLIBCXX__) && defined(UTS_DEBUG_CONTAINERS)
// This code is adapted from the Boost headers. It is probably a little fragile
template<typename Archive, typename T, typename Allocator>
inline void save(Archive &ar, const uts::vector<T, Allocator> &t, const unsigned int)
{
    boost::serialization::stl::save_collection<Archive, uts::vector<T, Allocator> >(ar, t);
}

template<typename Archive, typename T, typename Allocator>
inline void load(Archive &ar, uts::vector<T, Allocator> &t, const unsigned int)
{
    boost::serialization::stl::load_collection<
        Archive, uts::vector<T, Allocator>,
        boost::serialization::stl::archive_input_seq<
            Archive, uts::vector<T, Allocator> >,
        boost::serialization::stl::reserve_imp<uts::vector<T, Allocator> >
    >(ar, t);
}

template<typename Archive, typename T, typename Allocator>
inline void serialize(Archive &ar, uts::vector<T, Allocator> &t,
                      const unsigned int version)
{
    boost::serialization::split_free(ar, t, version);
}

template<typename Archive, typename T, typename Allocator>
inline void save(Archive &ar, const uts::list<T, Allocator> &t, const unsigned int)
{
    boost::serialization::stl::save_collection<Archive, uts::list<T, Allocator> >(ar, t);
}

template<typename Archive, typename T, typename Allocator>
inline void load(Archive &ar, uts::list<T, Allocator> &t, const unsigned int)
{
    boost::serialization::stl::load_collection<
        Archive, uts::list<T, Allocator>,
        boost::serialization::stl::archive_input_seq<
            Archive, uts::list<T, Allocator> >,
        boost::serialization::stl::no_reserve_imp<uts::list<T, Allocator> >
    >(ar, t);
}

template<typename Archive, typename T, typename Allocator>
inline void serialize(Archive &ar, uts::list<T, Allocator> &t,
                      const unsigned int version)
{
    boost::serialization::split_free(ar, t, version);
}

#endif

}} // namespace boost::serialization
