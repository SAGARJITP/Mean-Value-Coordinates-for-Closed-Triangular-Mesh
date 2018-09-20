

#include "colorInterpolation.h"


#include <stdio.h>
#include <stdlib.h>



//This function generates color for the vertices of the control mesh
std::unordered_map <YSHASHKEY, YsVec3> GenerateColorControlMesh(YsShellExt &mesh)
{

	//This map is to store the color for every vertex of the control mesh
	std::unordered_map <YSHASHKEY, YsVec3> colorMap;	

	YsVec3 bbx[2]; //The bounding box for the control mesh
	mesh.GetBoundingBox(bbx[0],bbx[1]); //Get the bounding box for the control mesh

	float dx = (bbx[1].xf() - bbx[0].xf()); //length of voxels in x direction
	float dy = (bbx[1].yf() - bbx[0].yf()); //length of voxels in y direction
	float dz = (bbx[1].zf() - bbx[0].zf()); //length of voxels in z direction

	for (auto vtHd = mesh.NullVertex(); true == mesh.MoveToNextVertex(vtHd);)
	{
		auto vtPos = mesh.GetVertexPosition(vtHd);
		auto key = mesh.GetSearchKey(vtHd);

		float r = (vtPos.xf() - bbx[0].xf())/dx;
		float g = (vtPos.yf() - bbx[0].yf())/dy;
		float b = (vtPos.yf() - bbx[0].zf())/dy;

		colorMap.insert({key,YsVec3(r,g,b)});
	}

	return colorMap;

}

//This function interpolates the color inside the control mesh (color of the polygons)
void InterpolateColor(const YsShellExt &Control_Mesh, YsShellExt &Model_Mesh, 
	const std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map, std::unordered_map <YSHASHKEY,YsVec3> CM_colorMap)
{


	std::unordered_map <YSHASHKEY,YsVec3> MM_colorMap; //color map for the vertices of the model mesh


	/////////Use mean value coordinates to interpolates color
	int i = 0;
	for (auto MM_Vertex_Hd = Model_Mesh.NullVertex(); true == Model_Mesh.MoveToNextVertex(MM_Vertex_Hd);)
	{			
		auto key = Model_Mesh.GetSearchKey(MM_Vertex_Hd);
		auto Weights = Weights_Map[i];

		YsVec3 TotalF(0.0, 0.0, 0.0);
		float TotalW = 0;
		for (auto CM_Vtx_Hd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(CM_Vtx_Hd);)
		{
			auto f = CM_colorMap.find(Control_Mesh.GetSearchKey(CM_Vtx_Hd))->second;
			TotalW += Weights.find(Control_Mesh.GetSearchKey(CM_Vtx_Hd))->second;
			TotalF += (Weights.find(Control_Mesh.GetSearchKey(CM_Vtx_Hd))->second)*f;
		}
		
		YsVec3 color = TotalF/TotalW; //interpolate the color for the model mesh vertex
		MM_colorMap.insert({key,color}); //Insert the color for the model mesh vertex in the map

		i++;
	}

	////////////change the color of the polygon based on the color of its vertices//////////
	for (auto plHd = Model_Mesh.NullPolygon(); true == Model_Mesh.MoveToNextPolygon(plHd);)
	{

		auto plVtHd=Model_Mesh.GetPolygonVertex(plHd); //the the handles of the vertices that make the polygon

		YsVec3 plColor(0.0,0.0,0.0);
		for (int i = 0; i < plVtHd.size(); i++)
		{
			auto vtKey = Model_Mesh.GetSearchKey(plVtHd[i]);
			plColor += MM_colorMap.find(vtKey)->second;
		}

		plColor = plColor/plVtHd.size(); //Get the polygon color

		Model_Mesh.SetPolygonColor(plHd,YsColor(plColor.xf(),plColor.yf(),plColor.zf(),1)); //Set the polygon color

	}

}



