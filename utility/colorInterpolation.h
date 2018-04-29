

#ifndef COLORINTERPOLATION_IS_INCLUDED
#define COLORINTERPOLATION_IS_INCLUDED

#include <vector>
#include <unordered_map>
#include "ysshellext.h"

//This function generates color for the vertices of the control mesh
std::unordered_map <YSHASHKEY, YsVec3> GenerateColorControlMesh(YsShellExt &mesh);


//This function interpolates the color inside the control mesh (color of the polygons)
void InterpolateColor(const YsShellExt &Control_Mesh, YsShellExt &Model_Mesh, 
	const std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map, std::unordered_map <YSHASHKEY,YsVec3> CM_colorMap);

#endif