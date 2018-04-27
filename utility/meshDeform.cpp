//This cpp file has function for scaling the control mesh and thus the model mesh accordingly 
//the mean value coordinates


#include "meshDeform.h"

#include <stdio.h>
#include <stdlib.h>


//This funtion takes the mesh and returns the vertex noormal for every vertex of the mesh as a map
std::unordered_map <YSHASHKEY,YsVec3> GetVertexNormals (const YsShellExt &mesh)
{

	std::unordered_map <YSHASHKEY, YsVec3> VertexNormals;

	//mesh.EnableSearch();

	for (auto vtHd = mesh.NullVertex(); true == mesh.MoveToNextVertex(vtHd);)
	{

		YsVec3 normal(0.0,0.0,0.0); //normal for the vertex vtHd;

		auto NeighPolygons = mesh.FindPolygonFromVertex(vtHd);
		

		//For all the neighbouring polygons of the vertex
		for (int i = 0; i < NeighPolygons.size(); i++)
		{
			auto polyNorm = mesh.GetNormal(NeighPolygons[i]); //Get the normal of the polygon
			normal += polyNorm; //summation of normals of the polygon
			
		}
		
		normal *= 1.0 / NeighPolygons.size();	
		normal.Normalize(); //Convert to unit vector
		
		VertexNormals.insert({mesh.GetSearchKey(vtHd),normal});

	}

	return VertexNormals; //Return the vertex normal map
}

//Get the centroid of the mesh
YsVec3 GetCentroid(const YsShellExt &mesh)
{

	YsVec3 centroid(0.0,0.0,0.0); //initilize to zero;

	for (auto vtHd = mesh.NullVertex(); true == mesh.MoveToNextVertex(vtHd);)
	{
		centroid += mesh.GetVertexPosition(vtHd);
	}

	centroid *= 1.0 / mesh.GetNumVertex();
	return centroid;

}

//This function scales up the mesh.. Use to scale up the control mesh
void ScaleUp(YsShellExt &mesh)
{

	YsVec3 centre_bef = GetCentroid(mesh);

	for (auto vtHd = mesh.NullVertex(); true ==  mesh.MoveToNextVertex(vtHd);)
	{
		mesh.SetVertexPosition(vtHd, 1.1*mesh.GetVertexPosition(vtHd));
	}

	YsVec3 centre_aft = GetCentroid(mesh);

	for (auto vtHd = mesh.NullVertex(); true ==  mesh.MoveToNextVertex(vtHd);)
	{
		mesh.SetVertexPosition(vtHd, mesh.GetVertexPosition(vtHd) + (centre_bef - centre_aft));
	}

}

//This function scales down the mesh...Use to scale down control mesh
void ScaleDown(YsShellExt &mesh)
{
	YsVec3 centre_bef = GetCentroid(mesh);

	for (auto vtHd = mesh.NullVertex(); true ==  mesh.MoveToNextVertex(vtHd);)
	{
		mesh.SetVertexPosition(vtHd, 0.9*mesh.GetVertexPosition(vtHd));
	}

	YsVec3 centre_aft = GetCentroid(mesh);

	for (auto vtHd = mesh.NullVertex(); true ==  mesh.MoveToNextVertex(vtHd);)
	{
		mesh.SetVertexPosition(vtHd, mesh.GetVertexPosition(vtHd) + (centre_bef - centre_aft));
	}

}

//This function change the control mesh...the vertices of the control mesh picked by the mouse
void MoveControlMesh(YsShellExt &Control_Mesh, const std::vector <YsShellExt::VertexHandle> PickedVertices,const YsVec3 &disp)
{
	if (!PickedVertices.empty()) //if set in not empty
	{
		for (auto &p : PickedVertices)
		{					
			auto vtHd = p;
			Control_Mesh.SetVertexPosition(vtHd, Control_Mesh.GetVertexPosition(vtHd) + disp);
		}
		
		
	}

}

//This function is used to deform the model mesh using mean value coordinates after changing the control mesh
void MoveModelMesh(const YsShellExt &Control_Mesh, YsShellExt &Model_Mesh, const std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map)
{


	int i = 0;
	for (auto MM_Vertex_Hd = Model_Mesh.NullVertex(); true == Model_Mesh.MoveToNextVertex(MM_Vertex_Hd);)
	{			
		auto Weights = Weights_Map[i];

		YsVec3 TotalF(0.0, 0.0, 0.0);
		float TotalW = 0;
		for (auto CM_Vtx_Hd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(CM_Vtx_Hd);)
		{
			TotalW += Weights.find(Control_Mesh.GetSearchKey(CM_Vtx_Hd))->second;
			TotalF += (Weights.find(Control_Mesh.GetSearchKey(CM_Vtx_Hd))->second)*Control_Mesh.GetVertexPosition(CM_Vtx_Hd);
		}

		//printf("vertex %d Old: %lf %lf %lf\n",i, Model_Mesh.GetVertexPosition(MM_Vertex_Hd).xf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).yf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).zf());
		YsVec3 NewPos = TotalF/TotalW; //Calculate new position for Model Mesh vertex
		Model_Mesh.SetVertexPosition(MM_Vertex_Hd,NewPos); //Set new position for the Model Mesh vertex
		//printf("vertex %d New: %lf %lf %lf\n",i , Model_Mesh.GetVertexPosition(MM_Vertex_Hd).xf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).yf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).zf());

		i++;
	}

}






