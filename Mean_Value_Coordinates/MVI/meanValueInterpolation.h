

#ifndef MEANVALUEINTERPOLATION_IS_INCLUDED
#define MEANVALUEINTERPOLATION_IS_INCLUDED


#include "ysclass.h"
#include "ysshellext.h"
#include <vector>
#include <unordered_map>


float Determinant(const YsVec3 &u0, const YsVec3 &u1, const YsVec3 &u2);

std::vector <std::unordered_map <YSHASHKEY,float>> GetMeanValueCoordinates(const YsShellExt &Model_Mesh,const YsShellExt &Control_Mesh);

std::unordered_map <YSHASHKEY,float> GetVertexMeanValues(const YsVec3 MM_Vertex_Pos, const YsShellExt &Control_Mesh);


#endif