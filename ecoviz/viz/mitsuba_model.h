/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  A. Peytavie  (adrien.peytavie@univ-lyon1.fr)
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

#ifndef MITSUBAMODEL_H
#define MITSUBAMODEL_H

#include <string>

struct MitsubaModel
{
    //double maxHeight;    // Maximum plant height to use this model
    std::string id;      // Instance id
    double height; // Actual height of the model
    double radius; // Actual height of the model
    bool isOpen;
};

#endif
