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
 * Data structure representing a rectangular region of 2D space.
 */

#ifndef UTS_COMMON_REGION_H
#define UTS_COMMON_REGION_H

#include <cstddef>
#include <boost/fusion/include/adapt_struct.hpp>


/**
 * A rectangle in a 2D space. When used to refer to pixels, (x0, y0) is
 * included by (x1, y1) are excluded. It is assumed that x0 <= x1,
 * y0 <= y1.
 */
struct Region
{
    int x0;    ///< Minimum x value
    int y0;    ///< Minimum y value
    int x1;    ///< One more than maximum x value
    int y1;    ///< One more than maximum y value

    /// Construct an empty region
    Region();

    /// Construct a region
    Region(int x0, int y0, int x1, int y1);

    /// Compute the bounding box of the union of two regions
    Region operator|(const Region &other) const;

    /// Compute the bounding box of the union of two regions
    Region &operator|=(const Region &other);

    /**
     * Compute the bounding box of the intersection of two regions.
     * If the intersection is empty, returns the default-constructed region.
     */
    Region operator&(const Region &other) const;

    /**
     * Compute the bounding box of the intersection of two regions.
     * If the intersection is empty, returns the default-constructed region.
     */
    Region &operator&=(const Region &other);

    bool operator==(const Region &other) const;
    bool operator!=(const Region &other) const;

    /// Return number of horizontal pixels
    int width() const;

    /// Return number of vertical pixels
    int height() const;

    /// Return true if the region contains no pixels
    bool empty() const;

    /// Return width times height
    std::size_t pixels() const;

    /**
     * Return the region expanded by a certain amount.
     * If the input is empty, the output is also empty.
     * The region may also be shrunk by passing a negative
     * dilation amount.
     */
    Region dilate(int border) const;

    /**
     * Equivalent to #dilate(-@a border).
     */
    Region erode(int border) const;

    /**
     * True if this region is a superset of @a other. This will
     * return true if @a other is an empty region, regardless of
     * the raw data members.
     */
    bool contains(const Region &other) const;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & x0;
        ar & y0;
        ar & x1;
        ar & y1;
    }
};

BOOST_FUSION_ADAPT_STRUCT(
    Region,
    (int, x0) (int, y0) (int, x1) (int, y1) )

#endif /* !UTS_COMMON_REGION_H */
