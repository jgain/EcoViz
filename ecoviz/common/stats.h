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
 * Utilities for recording statistics
 */

#ifndef UTS_COMMON_STATS_H
#define UTS_COMMON_STATS_H

#include <iostream>
#include <mutex>
#include <utility>
#include "debug_string.h"

/// Utilities for collecting execution statistics.
namespace stats
{

/// Implementation-specific details
namespace detail
{

/**
 * Implementation of @ref printAlways (terminal case).
 */
inline void printUnlocked()
{
}

/**
 * Implementation of @ref printAlways (non-terminal case).
 */
template<typename T, typename... Args>
void printUnlocked(T &&value, Args&&... args)
{
    std::cout << std::forward<T>(value);
    printUnlocked(std::forward<Args>(args)...);
}

std::mutex &getStatsPrintMutex();

bool getStatsEnabled();

} // namespace detail

/**
 * Enable recording of statistics with @ref print and @ref printStat.
 * @warning The enable flag is not thread-safe. It should be initialized before
 * any threads are created and not further altered.
 */
void enableStats(bool enable);

/**
 * Stream a sequence of items to @c std::cout. This functions takes a lock, so
 * concurrent calls from multiple threads are guaranteed not to be interleaved.
 * However, direct usage of @c std::cout is not locked out.
 */
template<typename... Args>
void printAlways(Args&&... args)
{
    std::lock_guard<std::mutex> guard(detail::getStatsPrintMutex());
    detail::printUnlocked(std::forward<Args>(args)...);
    std::cout.flush();
}

/**
 * Pass a sequence of items to @ref printAlways if statistics are enabled.
 */
template<typename... Args>
void print(Args&&... args)
{
    if (detail::getStatsEnabled())
        printAlways(std::forward<Args>(args)...);
}

/**
 * Print a named statistic, if statistics are enabled.
 */
template<typename T>
void printStat(const uts::string &name, T &&value)
{
    print("STAT,", name, ",", std::forward<T>(value), '\n');
}

} // namespace stats

#endif /* !UTS_COMMON_STATS_H */
