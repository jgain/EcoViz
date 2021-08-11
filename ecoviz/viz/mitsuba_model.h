#ifndef MITSUBAMODEL_H
#define MITSUBAMODEL_H

#include <string>

struct MitsubaModel
{
    double maxHeight;    // Maximum plant height to use this model
    std::string id;      // Instance id
    double actualHeight; // Actual height of the model
};

#endif