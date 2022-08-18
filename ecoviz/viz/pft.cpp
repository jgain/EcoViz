/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems (Undergrowth simulator)
 * Copyright (C) 2020  J.E. Gain  (jgain@cs.uct.ac.za)
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "pft.h"
#include <random>

GLfloat *Biome::getSpeciesColour(int specid)
{
    if (specid < pftypes.size())
        return pftypes.at(specid).basecol;

    // fallback
    return pftypes[0].basecol;
}

glm::vec4 Biome::getSpeciesColourV4(int specid)
{
    GLfloat *col = getSpeciesColour(specid);
    return glm::vec4(col[0], col[1], col[2], col[3]);
}

bool Biome::read_dataimporter(std::string cdata_fpath)
{
    // data_importer::common_data cdata = data_importer::common_data(cdata_fpath);
    cdata = new data_importer::common_data(cdata_fpath);

    return read_dataimporter((* cdata));
}

bool Biome::read_dataimporter(data_importer::common_data &cdata)
{
    pftypes.clear();
    species_info.clear();

    PFType pft;

    int maxspec_id = -1;

    cerr << "--- BIOME READ FROM DATABASE ---" << endl;

    for (auto &sppair : cdata.all_species)
    {
        data_importer::species &spec = sppair.second;

        // build meta data structure
        SpeciesInfo sinfo;
        sinfo.speciesId = spec.alpha_code;
        for (int i = 0; i < 4; i++)
            sinfo.species_color[i] = spec.basecol[i];
        sinfo.species_name = spec.cname;
        sinfo.scientific_name = spec.sname;
        sinfo.species_num_id = species_info.size();
        species_info.push_back(sinfo);

        pft.code = spec.cname;
        for (int i = 0; i < 4; i++)
            pft.basecol[i] = spec.basecol[i];
        pft.draw_hght = spec.draw_hght;
        pft.draw_radius = spec.draw_radius;
        pft.draw_box1 = spec.draw_box1;
        pft.draw_box2 = spec.draw_box2;
        switch (spec.shapetype)
        {
            case (data_importer::treeshape::BOX):
                pft.shapetype = TreeShapeType::BOX;
                break;
            case (data_importer::treeshape::CONE):
                pft.shapetype = TreeShapeType::CONE;
                break;
            case (data_importer::treeshape::SPHR):
                pft.shapetype = TreeShapeType::SPHR;
                break;
            case (data_importer::treeshape::INVCONE):
                pft.shapetype = TreeShapeType::INVCONE;
                break;
            case (data_importer::treeshape::HEMISPHR):
                pft.shapetype = TreeShapeType::HEMISPHR;
                break;
            case (data_importer::treeshape::CYL):
                pft.shapetype = TreeShapeType::CYL;
                break;
            default:
                assert(false);
                break;
        }

        pftypes.push_back(pft);

        if (sppair.first > maxspec_id)
            maxspec_id = sppair.first;

    }
    cerr << "Number of species assigned = " << (int) pftypes.size() << endl;

    std::default_random_engine gen(25);
    std::uniform_real_distribution<float> unif;
    for (int i = 0; i < 48; i++)
    {
        pft.code = "i" + std::to_string(i);
        for (int i = 0; i < 3; i++)
            pft.basecol[i] = unif(gen);
        pft.basecol[3] = 1.0f;
        pft.draw_hght = 0.1f;
        pft.draw_radius = 0.1f;
        pft.draw_box1 = 1.0f;
        pft.draw_box2 = 1.0f;
        int shapeint = unif(gen) * 4;
        switch (shapeint)
        {
            case 0:
                pft.shapetype = TreeShapeType::BOX;
                break;
            case 1:
                pft.shapetype = TreeShapeType::CONE;
                break;
            case 2:
                pft.shapetype = TreeShapeType::SPHR;
                break;
            case 3:
                pft.shapetype = TreeShapeType::INVCONE;
                break;
            default:
                assert(false);
                break;
        }

        pftypes.push_back(pft);

    }
    return true;
}
