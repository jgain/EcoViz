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
 * OBJ format export.
 */

#ifndef UTS_COMMON_OBJ_H
#define UTS_COMMON_OBJ_H

#include <common/debug_string.h>
#include "map.h"

/**
 * Save an OBJ file.
 */
void writeOBJ(const uts::string &filename, const MemMap<height_tag> &map, const Region &region);

#endif /* !UTS_COMMON_TERRAGEN_H */
