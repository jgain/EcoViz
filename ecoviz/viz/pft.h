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


#ifndef PFT
#define PFT
/* file: pft.h
   author: (c) James Gain, 2018
   notes: plant functional type database
*/

#include "terrain.h"
#include <data_importer/data_importer.h>

enum TreeShapeType
{
    SPHR,   //< sphere shape, potentially elongated
    BOX,    //< cuboid shape
    CONE,    //< cone shape
    INVCONE, //< inverted cone
    HEMISPHR, //< hemisphere
    CYL       //< cylinder
};

struct PFType
{
    string code;        //< a mnemonic for the plant type
    
    //< rendering parameters
    GLfloat basecol[4];  //< base colour for the PFT, individual plants will vary
    float draw_hght;    //< canopy height scaling
    float draw_radius;  //< canopy radius scaling
    float draw_box1;    //< box aspect ratio scaling
    float draw_box2;    //< box aspect ration scaling
    TreeShapeType shapetype; //< shape for canopy: sphere, box, cone

};

// meta data for a species
struct SpeciesInfo
{
    int species_num_id; // numeric ID (index)
    std::string speciesId; // 4-letter code
    std::string species_name; // species name
    std::string scientific_name; // scientific species name
    float species_color[4]; // RGBA hex color code
};

class Biome
{
private:
    std::vector<PFType> pftypes; //< vector of plant functional types in the biome
    std::vector<SpeciesInfo> species_info;
    std::string name; //< biome name
    data_importer::common_data * cdata; // access to pdb database

    
public:

    Biome(){ cdata = nullptr; }

    ~Biome(){ pftypes.clear(); delete cdata; }

    /// numPFTypes: returns the number of plant functional types in the biome
    int numPFTypes(){ return (int) pftypes.size(); }

    /// getPFType: get the ith plant functional type in the biome
    PFType * getPFType(int i){ return &pftypes[i]; }

    bool read_dataimporter(std::string cdata_fpath);
    bool read_dataimporter(data_importer::common_data &cdata);

    GLfloat *getSpeciesColour(int specid);
    glm::vec4 getSpeciesColourV4(int specid);

    const std::vector<SpeciesInfo> &getSpeciesMetaData() { return species_info; }
};



#endif
