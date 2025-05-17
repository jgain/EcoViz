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
 * Misc string utilities.
 */

#ifndef COMMON_STR_H
#define COMMON_STR_H

#include "debug_string.h"

/// Tests whether @a suffix is a suffix of @a a
static inline bool endsWith(const uts::string &a, const uts::string &suffix)
{
    return a.size() >= suffix.size() && a.substr(a.size() - suffix.size()) == suffix;
}

#endif /* !COMMON_STR_H */
