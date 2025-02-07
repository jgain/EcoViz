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


//
// TypeMap
//

#include "cohortmaps.h"
#include "typemap.h"
#include "vecpnt.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <time.h>

#include <QFileInfo>
#include <QLabel>
#include <QDir>
#include <QResource>

/*
Perceptually uniform colourmaps from:
http://peterkovesi.com/projects/colourmaps/
*/

using namespace std;

// Sonoma County Colours
float hardwood[] = {0.749f, 0.815f, 0.611f, 1.0f};
float conifer[] = {0.812f, 0.789f, 0.55f, 1.0f};
float mixed[] = {0.552f, 0.662f, 0.533f, 1.0f};
float riparian[] = {0.4f, 0.6f, 0.6f, 1.0f};
float nonnative[] = {0.7, 0.6, 0.4, 1.0f};
float sliver[] = {0.652f, 0.762f, 0.633f, 1.0f};
float shrubery[] = {0.882f, 0.843f, 0.713f, 1.0f};
float ripshrub[] = {0.509f, 0.67f, 0.584f, 1.0f};
float herb[] = {0.75f, 0.7f, 0.7f, 1.0f};
float herbwet[] = {0.623f, 0.741f, 0.825f, 1.0f};
float aquatic[] = {0.537f, 0.623f, 0.752f, 1.0f};
float salt[] = {0.727f, 0.763f, 0.534f, 1.0f};
float barrenland[] = {0.818f, 0.801f, 0.723f, 1.0f};
float agriculture[] = {0.894f, 0.913f, 0.639f, 1.0f};
float wet[] = {0.737f, 0.823f, 0.952f, 1.0f};
float developed[] = {0.5f, 0.4f, 0.5f, 1.0f};


// palette colours

float freecol[] = {0.755f, 0.645f, 0.538f, 1.0f};
float sparseshrub[] = {0.814f, 0.853f, 0.969f, 1.0f};
float sparsemed[] = {0.727f, 0.763f, 0.834f, 1.0f};
float sparsetall[] = {0.537f, 0.623f, 0.752f, 1.0f};
float denseshrub[] = {0.749f, 0.815f, 0.611f, 1.0f};
float densemed[] = {0.552f, 0.662f, 0.533f, 1.0f};
float densetall[] = {0.300f, 0.515f, 0.1f, 1.0f};


// default colours
float barren[] = {0.818f, 0.801f, 0.723f, 1.0f};            // 1
float ravine[] = {0.755f, 0.645f, 0.538f, 1.0f};            // 2
float canyon[] = {0.771f, 0.431f, 0.351f, 1.0f};            // 3
float grassland[] = {0.552f, 0.662f, 0.533f, 1.0f};         // 4
float pasture[] = {0.894f, 0.913f, 0.639f, 1.0f};           // 5
float foldhills[] = {0.727f, 0.763f, 0.534f, 1.0f};         // 6
float orchard[] = {0.749f, 0.815f, 0.611f, 1.0f};           // 7
float evergreenforest[] = {0.300f, 0.515f, 0.0f, 1.0f};     // 8
float otherforest[] = {0.552f, 0.662f, 0.533f, 1.0f};       // 9
float woodywetland[] = {0.509f, 0.67f, 0.584f, 1.0f};       // 10
float herbwetland[] = {0.623f, 0.741f, 0.825f, 1.0f};       // 11
float frillbank[] = {0.4f, 0.6f, 0.6f, 1.0f};               // 12
float shrub[] = {0.882f, 0.843f, 0.713f, 1.0f};             // 13
float flatinterest[] = {0.812f, 0.789f, 0.55f, 1.0f};      // 14
float water[] = {0.737f, 0.823f, 0.952f, 1.0f};            // 15
float special[] = {0.4f, 0.4f, 0.4f, 1.0f};                 // 16
float extra[] = {0.5f, 0.4f, 0.5f, 1.0f};                   // 17
float realwater[] = {0.537f, 0.623f, 0.752f, 1.0f};         // 18
float boulders[] = {0.671f, 0.331f, 0.221f, 1.0f};          // 19

// transect colour
float redcol[] = {1.0f, 0.5f, 0.5f, 1.0f};

//// DATAMAPS ////

void DataMaps::initMaps(int numYears, int numMaps, int dx, int dy)
{
    dimx = dx; dimy = dy;
    for(int y = 0; y < numYears; y++)
    {
        std::vector<basic_types::MapFloat *> yearmaps;
        for(int m = 0; m < numMaps; m++)
        {
            basic_types::MapFloat * map = new basic_types::MapFloat();
            map->setDim(dimx, dimy);
            map->fill(-1.0f);
            yearmaps.push_back(map);
        }
        dmaps.push_back(yearmaps);
    }
}

void DataMaps::clearMaps()
{
    for(auto yearmaps: dmaps)
    {
        for(auto map: yearmaps)
            if(map != nullptr)
                delete map;
        yearmaps.clear();
    }
    dmaps.clear();
}

bool DataMaps::indexToLoc(basic_types::MapInt & idxmap, int idx, int & x, int & y)
{
    bool found = false, fin = false;
    int dimx, dimy;

    x = 0; y = 0;
    idxmap.getDim(dimx, dimy);

    // convert to hashmap if this starts impacting performance, which is unlikely given the size of data cells
    while(!found && !fin)
    {
        found = (idxmap.get(x,y) == idx);
        if(!found)
        {
            x++;
            if(x == dimx)
            {
                y++; x = 0;
            }
        }
        fin = (y == dimy);
    }
    return found;
}

bool DataMaps::loadIndexMap(const std::string & idxfilename, basic_types::MapInt & idxmap)
{
    int width, height, xcorner, ycorner, idx;
    ifstream infile;
    float val, step, nodata;
    std::string skp;

    infile.open((char *) idxfilename.c_str(), ios_base::in);
    if(infile.is_open())
    {
        infile >> skp >> width >> skp >> height;
        infile >> skp >> xcorner >> skp >> ycorner;
        infile >> skp >> step;
        infile >> skp >> nodata;

        idxmap.setDim(height, width);
        idxmap.fill(-1); // set to empty

         for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
            {
                infile >> val;
                if (val < minuszero) // empty index = -1, should probably test for no-data value instead
                    idx = -1;
                else
                    idx = (int) val;
                idxmap.set(y,x,idx); // orientation swap to match internal terrain representation
            }
        infile.close();

        return true;
    }
    else
    {
        cerr << "Error DataMaps::loadIndexMap: unable to open file" << idxfilename << endl;
        return false;
    }
}

float DataMaps::getTerVal(int year, int midx, int tx, int ty)
{
    if(tx < cover.x0 || tx >= cover.x1 || ty < cover.y0 || ty >= cover.y1 ) // out of bounds
    {
        // cerr << "OOB " << tx << ", " << ty << " cover = " << cover.x0 << ", " << cover.y0 << " -> " << cover.x1 << ", " << cover.y1 << endl;
        return -1.0f; // empty value
    }

    // find coordinates in datamap space
    float ex = (float) (cover.x1 - cover.x0);
    float ey = (float) (cover.y1 - cover.y0);
    // normalized coordinates
    float x = (float) (tx - cover.x0) / ex;
    float y = (float) (ty - cover.y0) / ey;

    // coord swap
    int mx = (int) (x * (float) dimx);
    int my = (int) (y * (float) dimy);
    return dmaps[year][midx]->get(mx, my);
}

bool DataMaps::loadDataMaps(const std::string &idxfilename, const std::string &datafilename, int numyears)
{
     basic_types::MapInt idxmap;
     ifstream infile;
     int nummaps, numcols, year, idx, dx, dy, x, y;
     float val;
     string str;

     if(loadIndexMap(idxfilename, idxmap))
     {
         // indices loaded now turn to the data
         infile.open((char *) datafilename.c_str(), ios_base::in);
         if(infile.is_open())
         {
             infile >> nummaps;

             // setup dmaps structure based on number of years and number of maps
             numcols = nummaps+2;

             // initialize data maps
             idxmap.getDim(dx, dy);
             initMaps(numyears, nummaps, dx, dy);

             // get names of maps
             for(int col = 0; col < numcols; col++)
             {
                 if(col < numcols-1) // all but last are comma terminated
                     getline(infile, str, ',');
                 else // last is newline terminated
                     getline(infile, str);
                 if(col >= 2) // discard entries for year and index
                 {
                     dnames.push_back(str);
                     dmax.push_back(0.0f);
                 }
             }

             // read in data ranges

             // get map data
             while(infile.peek() != EOF)
             {
                 // process year string already read
                 getline(infile, str, ','); // prime the pump
                 year = stoi(str);

                 // map index
                 getline(infile, str, ',');
                 idx = stoi(str);

                 // data fields
                 for(int col = 0; col < nummaps; col++)
                 {
                     if(col < nummaps-1) // all but last are comma terminated
                         getline(infile, str, ',');
                     else
                         getline(infile, str);
                     val = stof(str);

                     if(indexToLoc(idxmap, idx, x, y)) // search in the index map for the correct entry
                     {
                        if(year < numyears)
                        {
                            if(val > dmax[col])
                                dmax[col] = val;
                            dmaps[year][col]->set(x, y, val);
                        }
                     }
                     // else
                     // {
                     //     cerr << "Error DataMaps::loadDataMaps: index " << idx << " not found in index map " << idxfilename << endl;
                     // }
                     // else
                     //     cerr << "Error DataMaps::loadDataMaps: more years in " << datafilename << " than in the rest of the data" << endl;
                 }
             }
             infile.close();
             return true;
         }
         else
         {
             cerr << "Error DataMaps::loadDataMaps: unable to open file" << datafilename << endl;
             return false;
         }

     }
     else
         return false;
}

void DataMaps::extractRegion(int year, int midx, Region superRegion, Region subRegion, basic_types::MapFloat * subMap)
{
    cover = superRegion;
    subMap->setDim(subRegion.width(), subRegion.height());

    for(int x = subRegion.x0; x < subRegion.x1; x++)
          for(int y = subRegion.y0; y < subRegion.y1; y++)
          {
              float val = getTerVal(year, midx, x, y); // internal map oriented differently to region
              subMap->set(x-subRegion.x0, y-subRegion.y0, val);
          }
}

//// TYPEMAP ////

TypeMap::TypeMap(TypeMapType purpose)
{
    tmap = new basic_types::MapInt;
    setPurpose(purpose);
}

TypeMap::TypeMap(int w, int h, TypeMapType purpose)
{
    tmap = new basic_types::MapInt;
    matchDim(w, h);
    setPurpose(purpose);
}

TypeMap::~TypeMap()
{
    delete tmap;
    for(int i = 0; i < (int) colmap.size(); i++)
        delete [] colmap[i];
    colmap.clear();
}

void TypeMap::clear()
{
    tmap->fill(0);
}

void TypeMap::initPaletteColTable()
{
    GLfloat *col;

    for(int i = 0; i < 32; i++) // set all colours in table to black initially
    {
        col = new GLfloat[4];
        col[0] = col[1] = col[2] = 0.0f; col[3] = 1.0f;
        colmap.push_back(col);
    }

    numSamples = 7;

    colmap[0] = freecol;
    colmap[1] = sparseshrub;
    colmap[2] = sparsemed;
    colmap[3] = sparsetall;
    colmap[4] = denseshrub;
    colmap[5] = densemed;
    colmap[6] = densetall;
}

void TypeMap::initTransectColTable()
{
    GLfloat * col;
    for(int i = 0; i < 6; i++) // set all colours in table to black initially
    {
        col = new GLfloat[4];
        col[0] = col[1] = col[2] = 0.0f; col[3] = 1.0f;
        colmap.push_back(col);
    }

    numSamples = 6;

    colmap[0] = freecol;
    colmap[1] = freecol;
    colmap[2] = redcol;
}

int TypeMap::getNumSamples()
{
    return numSamples;
}

void TypeMap::initNaturalColTable()
{
    GLfloat *col;

    for(int i = 0; i < 32; i++) // set all colours in table to black initially
    {
        col = new GLfloat[4];
        col[0] = col[1] = col[2] = 0.0f; col[3] = 1.0f;
        colmap.push_back(col);
    }

    // saturated prime colours and combos
    /*
     (colmap[1])[0] = 1.0f; // red
     (colmap[2])[1] = 1.0f; // green
     (colmap[3])[2] = 1.0f; // blue
     (colmap[4])[1] = 1.0f; (colmap[4])[2] = 1.0f; // cyan
     (colmap[5])[0] = 1.0f; (colmap[5])[1] = 1.0f; // yellow
     (colmap[6])[0] = 1.0f; (colmap[6])[2] = 1.0f; // magenta
     (colmap[7])[0] = 0.5f;  (colmap[7])[1] = 0.5f; (colmap[7])[2] = 0.5f; // grey
     (colmap[8])[1] = 0.5f; (colmap[8])[2] = 0.5f; // teal
     */

    numSamples = 20;

    // default
    //colmap[0] = c0;
    colmap[1] = barren;
    colmap[2] = ravine;
    colmap[3] = canyon;
    colmap[4] = grassland;
    colmap[5] = pasture;
    colmap[6] = foldhills;
    colmap[7] = orchard;
    colmap[8] = woodywetland;
    colmap[9] = otherforest;
    colmap[10] = woodywetland;
    colmap[11] = herbwetland;
    colmap[12] = frillbank;
    colmap[13] = shrub;
    colmap[14] = flatinterest;
    colmap[15] = water;
    colmap[16] = special;
    colmap[17] = extra;
    colmap[18] = realwater;
    colmap[19] = boulders;
}

void TypeMap::initPerceptualColTable(std::string colmapfile, int samples, float truncend)
{
    GLfloat *col;
    float r[256], g[256], b[256];
    ifstream infile;
    QString valstr, line;
    int i, pos, step;

    if(samples < 3 || samples > 32)
        cerr << "Error: sampling of colour map must be in the range [3,32]" << endl;

    for(i = 0; i < 32; i++) // set all colours in table to black initially
    {
        col = new GLfloat[4];
        col[0] = col[1] = col[2] = 0.0f; 
        col[3] = 1.0f;
        colmap.push_back(col);
    }

    // input is a csv file, with 256 RGB entries, one on each line
    // note that this is not robust to format errors in the input file
    QFile file(colmapfile.c_str()); // Use of the ressources
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qWarning() << "Failed to open Colour file!" << colmapfile;
      numSamples = -1;
      return;
    }
    else
    {
      qDebug() << "Colour file opened successfully!";
    }

    QTextStream in(&file);
    i = 0;

    while (!in.atEnd() && i < 256) // Éviter de dépasser le tableau
    {
      line = in.readLine();
      QStringList values = line.split(',');

      if (values.size() >= 3)
      {
        r[i] = values[0].toFloat();
        g[i] = values[1].toFloat();
        b[i] = values[2].toFloat();
      }
      i++;
    }

    file.close();
    numSamples = samples + 1;

    // Échantillonne la table de couleurs
    step = static_cast<int>((256.0f * truncend) / (samples - 1));
    pos = 0;

    for (i = 1; i <= samples; i++)
    {
      colmap[i][0] = static_cast<GLfloat>(r[pos]);
      colmap[i][1] = static_cast<GLfloat>(g[pos]);
      colmap[i][2] = static_cast<GLfloat>(b[pos]);
      pos += step;
    }
}

void TypeMap::clipRegion(Region &reg)
{
    if(reg.x0 < 0) reg.x0 = 0;
    if(reg.y0 < 0) reg.y0 = 0;
    if(reg.x1 > width()) reg.x1 = width();
    if(reg.y1 > height()) reg.y1 = height();
}

void TypeMap::matchDim(int w, int h)
{
    int mx, my;

    mx = tmap->width();
    my = tmap->height();

    // if dimensions don't match then reallocate
    if(w != mx || h != my)
    {
        dirtyreg = Region(0, 0, w, h);
        tmap->setDim(w, h);
        tmap->fill(0); // set to empty type
    }
}

void TypeMap::replaceMap(basic_types::MapInt * newmap)
{
    assert(tmap->width() == newmap->width());
    assert(tmap->height() == newmap->height());
    for (int y = 0; y < tmap->height(); y++)
        for (int x = 0; x < tmap->width(); x++)
            tmap->set(y,x, newmap->get(y,x));
}

int TypeMap::load(const std::string &filename, TypeMapType purpose)
{
    int tp, maxtp = 0; // mintp = 100;
    int width, height;
    ifstream infile;
    float val, maxval = 0.0f, range;

    infile.open((char *) filename.c_str(), ios_base::in);
    if(infile.is_open())
    {
        infile >> width >> height;
        // cerr << "width = " << width << " height = " << height << endl;
        matchDim(height, width);
        // convert to internal type map format

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                switch(purpose)
                {
                    case TypeMapType::EMPTY: // do nothing
                        break;
                    case TypeMapType::TRANSECT: // do nothing
                        break;   
                    case TypeMapType::GREYRAMP:
                    case TypeMapType::HEATRAMP:
                    case TypeMapType::BLUERAMP:
                        infile >> val;
                        if(val > maxval)
                            maxval = val;

                        // discretise into ranges of water values
                        range = 1000.0f;
                        // clamp values to range
                        if(val < 0.0f) val = 0.0f;
                        if(val > range) val = range;
                        tp = (int) (val / (range+pluszero) * (numSamples-1))+1;
                        break;
                    default:
                        break;
                }
                tmap->set(y,x,tp);

                if(tp > maxtp)
                    maxtp = tp;
                /*
                if(tp < mintp)
                    mintp = tp;
                */

            }
        }
        infile.close();
        // cerr << "maxtp = " << maxtp << endl;
        // cerr << "mintp = " << mintp << endl;
    }
    else
    {
        cerr << "Error TypeMap::loadTxt: unable to open file" << filename << endl;
    }
    return maxtp;
}

bool TypeMap::loadCategoryImage(const std::string &filename)
{
    int width, height;
    QImage img(QString::fromStdString(filename)); // load image from file

    QFileInfo check_file(QString::fromStdString(filename));

    if(!(check_file.exists() && check_file.isFile()))
        return false;

    // set internal storage dimensions
    width = img.width();
    height = img.height();
    matchDim(height, width);

    // convert to internal type map format
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
        {
            QColor col = img.pixelColor(x, y);
            int r, g, b;
            col.getRgb(&r, &g, &b); // all channels store the same info so just use red
            tmap->set(y,x, r - 100); // convert greyscale colour to category index
        }
    return true;
}

int TypeMap::convert(basic_types::MapFloat * map, TypeMapType purpose, float range)
{
    int tp, maxtp = 0;
    int width, height;
    float val;

    map->getDim(width, height);
    matchDim(height, width);
    // convert to internal type map format
    int mincm, maxcm;
    mincm = 100; maxcm = -1;

    for(int x = 0; x < width; x++)
        for(int y = 0; y < height; y++)
        {
            tp = 0;
            switch(purpose)
            {
                case TypeMapType::EMPTY: // do nothing
                    break;
                case TypeMapType::TRANSECT: // do nothing
                    // two values
                    val = map->get(x, y);
                    if(val > 0.001f)
                        tp = 2;
                    else
                        tp = 0;
                    break;
                case TypeMapType::GREYRAMP:
                case TypeMapType::HEATRAMP:
                case TypeMapType::BLUERAMP:
                    val = map->get(x, y);

                    // discretise into ranges of water values
                    // clamp values to range
                    if(val < 0.0f) // empty cell
                    {
                        tp = 0;
                    }
                    else
                    {
                        if(val > range)
                            val = range;
                        tp = (int) (val / (range+pluszero) * (numSamples-2)) + 1;
                    }
                    break;
                default:
                    break;
            }
            tmap->set(y,x,tp);

            if(tp > maxtp)
                maxtp = tp;
        }
    return maxtp;
}

void TypeMap::save(const std::string &filename)
{
    ofstream outfile;

    outfile.open((char *) filename.c_str(), ios_base::out);
    if(outfile.is_open())
    {
        outfile << width() << " " << height() << endl;

        // dimensions
        for (int x = 0; x < width(); x++)
            for (int y = 0; y < height(); y++)
            {
                outfile << get(x, y) << " ";
            }
        outfile.close();
    }
    else
    {
        cerr << "Error TypeMap::save: unable to write to file" << endl;
    }
}

void TypeMap::saveToPaintImage(const std::string &filename)
{
    unsigned char * mask = new unsigned char[tmap->width()*tmap->height()];
    int i = 0;

    cerr << "paint file: " << filename << endl;

    //mask.resize(tmap->width()*tmap->height(), 0.0f);
    for (int x = 0; x < tmap->width(); x++)
        for (int y = 0; y < tmap->height(); y++)
        {
            switch(tmap->get(x,y)) // check order
            {
            case 0:
                mask[i] = 0;
                break;
            case 1: // sparse low
                mask[i] = 38;
                break;
            case 2: // sparse med
                mask[i] = 76;
                break;
            case 3: // sparse tall
                mask[i] = 115;
                break;
            case 4: // dense low
                mask[i] = 153;
                break;
            case 5: // dense med
                mask[i] = 191;
                break;
            case 6: // dense tall
                mask[i] = 230;
                break;
            default:
                mask[i] = 0;
            }
            i++;
        }

    // use QT image save functions
    QImage img;
    img = QImage(mask, tmap->width(), tmap->height(), QImage::Format_Grayscale8);
    img.save(QString::fromStdString(filename), "PNG", 100);
    delete [] mask;
}

void TypeMap::setPurpose(TypeMapType purpose)
{

    usage = purpose;
    switch(usage)
    {
        case TypeMapType::EMPTY:
            initPaletteColTable();
            break;
        case TypeMapType::TRANSECT:
            initTransectColTable();
            break;
        case TypeMapType::GREYRAMP:
            initPerceptualColTable(":/resources/colourmaps/linear_grey_10-95_c0_n256.csv", 10);
            break;
        case TypeMapType::HEATRAMP:
            initPerceptualColTable(":/resources/colourmaps/linear_kryw_5-100_c67_n256.csv", 10);
            break;
        case TypeMapType::BLUERAMP:
            initPerceptualColTable(":/resources/colourmaps/linear_blue_95-50_c20_n256.csv", 10);
            break;
        default:
            break;
    }
}

void TypeMap::resetType(int ind)
{
    // wipe all previous occurrences of ind
    #pragma omp parallel for
    for(int j = 0; j < tmap->height(); j++)
        for(int i = 0; i < tmap->width(); i++)
            if(tmap->get(j,i) == ind)
                tmap->set(j,i,0);
    dirtyreg.x0 = 0; dirtyreg.y0 = 0;
    dirtyreg.x1 = tmap->width(); dirtyreg.y1 = tmap->height();
}

void TypeMap::setColour(int ind, GLfloat * col)
{
    for(int i = 0; i < 4; i++)
        colmap[ind][i] = col[i];
}
