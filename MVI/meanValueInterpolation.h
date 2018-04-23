

#ifndef MEANVALUEINTERPOLATION_IS_INCLUDED
#define MEANVALUEINTERPOLATION_IS_INCLUDED


#include "ysclass.h"
#include "ysshellext.h"
#include <vector>
#include <unordered_map>


float Determinant(YsVec3 &u0, YsVec3 &u1, YsVec3 &u2);

std::vector <std::unordered_map <YSHASHKEY,float>> GetMeanValueCoordinates(YsShellExt &Model_Mesh, YsShellExt &Control_Mesh);

std::unordered_map <YSHASHKEY,float> GetVertexMeanValues(const YsVec3 MM_Vertex_Pos, const YsShellExt &Control_Mesh);




#endif