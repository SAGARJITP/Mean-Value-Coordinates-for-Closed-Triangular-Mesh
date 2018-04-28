
#ifndef SCALING_IS_INCLUDED
#define SCALING_IS_INCLUDED


#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <ysclass.h>
#include "ysshellext.h"

//This funtion takes the mesh and returns the vertex noormal for every vertex of the mesh as a map
std::unordered_map <YSHASHKEY,YsVec3> GetVertexNormals (const YsShellExt &mesh);

//This function is use to get the centre of mesh
YsVec3 GetCentroid(const YsShellExt &mesh);

//This function scales up the mesh..use to scale up the control mesh
void ScaleUp(YsShellExt &mesh);

//This function scales down the mesh...use to scale down the control mesh
void ScaleDown(YsShellExt &mesh); 

//This function change the control mesh...the vertices of the control mesh picked by the mouse
void MoveControlMesh(YsShellExt &Control_Mesh, const std::unordered_set <YSHASHKEY> PickedVertices,const YsVec3 &disp);

//This function is used to deform the model mesh using mean value coordinates after changing the control mesh
void MoveModelMesh(const YsShellExt &Control_Mesh,YsShellExt &Model_Mesh, const std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map);




#endif