#include "Mesh.h"

using namespace FEM;

Mesh::Mesh()
{ 
    mBBMin[0] = mBBMin[1] = mBBMin[2] = std::numeric_limits<double>::max(); 
    mBBMax[0] = mBBMax[1] = mBBMax[2] = -std::numeric_limits<double>::max();
};