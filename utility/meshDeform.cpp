//This cpp file has function for scaling the control mesh and thus the model mesh accordingly 
//the mean value coordinates


#include "meshDeform.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define INF_D (std::numeric_limits<float>::infinity())


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
void MoveControlMesh_vertex(YsShellExt &Control_Mesh, const std::unordered_set <YSHASHKEY> PickedVertices,const YsVec3 &disp)
{
	if (!PickedVertices.empty()) //if set in not empty
	{
		Control_Mesh.EnableSearch();
		for (auto &p : PickedVertices)
		{					
			auto vtHd = Control_Mesh.FindVertex(p);
			Control_Mesh.SetVertexPosition(vtHd, Control_Mesh.GetVertexPosition(vtHd) + disp);
		}
				
	}

}

//This function moves the control messh by means of cluster......formed by k means clustering
void MoveControlMesh_cluster(YsShellExt &Control_Mesh, std::unordered_map <int,YsVec3> &K_Points, const std::unordered_map <YSHASHKEY,int> &K_Groups,const int PickedPoint,const YsVec3 &disp)
{

	Control_Mesh.EnableSearch();

	if (PickedPoint != -1)
	{
		//LOOP through all the vertices of the control mesh
		for (auto &v : K_Groups)
		{
			//if belong to the same cluster
			if (v.second == PickedPoint)
			{
				auto vtHd = Control_Mesh.FindVertex(v.first);
				Control_Mesh.SetVertexPosition(vtHd,Control_Mesh.GetVertexPosition(vtHd) + disp);
			}
		}

		K_Points.find(PickedPoint)->second += disp; //move the cluster point as well
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

//K Mean clustering.. This function groups the vertices of the Control_Mesh into k groups using k-means clustering
void K_Means(std::unordered_map <int,YsVec3> &K_Points,std::unordered_map <YSHASHKEY,int> &K_Groups,YsShellExt &Control_Mesh,int k)
{
	YsVec3 bbx[2];
	Control_Mesh.GetBoundingBox(bbx[0],bbx[1]); //Get the bounding box for control mesh

	//Initialize all the groups to zero
	int n = 0;
	for (auto vtHd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(vtHd);)
	{
		K_Groups.insert({Control_Mesh.GetSearchKey(vtHd),n});
		n++;
	}

	srand(time(NULL));

	double Tolerance = 0.05; //Tolerance for stopiing the k_means iteration

	
	//printf("x range %lf %lf\n", bbx[0].xf() , bbx[1].xf());
	//printf("y range %lf %lf\n", bbx[0].yf() , bbx[1].yf());
	//printf("z range %lf %lf\n", bbx[0].zf() , bbx[1].zf());
	

	//Randomly generate k points within the bounding box
	float x,y,z;
	for (int i = 0; i < k ; i++)
	{
		x = rand() % int(100*(bbx[1].xf() - bbx[0].xf())) + 100*bbx[0].xf(); //generate random number for x
		y = rand() % int(100*(bbx[1].yf() - bbx[0].yf())) + 100*bbx[0].yf(); //generate random number for y
		z = rand() % int(100*(bbx[1].zf() - bbx[0].zf())) + 100*bbx[0].zf(); //generate random number for z

		x /= 100;
		y /= 100;
		z /= 100; 
		
		K_Points.insert({i,YsVec3(x,y,z)}); //insert the point in K_Points		
		//printf("%lf %lf %lf\n",x,y,z);
	}



	//Group the vertices of the Control_Mesh iteratively
	bool terminate  = false; //stop when terminate  = 1;	
	while(!terminate) //continue till terminate is false
	//for (int i = 0; i < 100; i++)
	{

		for (auto vtHd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(vtHd);)
		{
			YsVec3 vtPos = Control_Mesh.GetVertexPosition(vtHd); //Grt the coordinates of the vertex
			auto vtKey = Control_Mesh.GetSearchKey(vtHd); //Get the unique search key of the vertex
			float Min_Dist = INF_D; //Initialize min diatance to inf
			float dist;

			//For every points in k groups
			for (auto &k : K_Points )
			{
				auto k_point = k.second; //get the point of the k_groups
				dist = (vtPos - k_point).GetLength();

				if (dist < Min_Dist)
				{
					Min_Dist = dist;
					K_Groups.find(vtKey)->second = k.first;
				}

			}
		}
		//Update the k points
		terminate = Update_KPoints(Control_Mesh, K_Points, K_Groups);

	}


	/*
	printf("groups.....\n");
	for (auto &p : K_Groups)
	{
		printf("%d %d\n",p.first,p.second);
	}	
	printf("Points........\n");
	for (auto k : K_Points)
	{
		auto pos = k.second;
		printf("point %d: %lf %lf %lf\n",k.first,pos.xf(),pos.yf(),pos.zf());
	}
	*/
	
	
	//printf(".......\n");

}


//This function updates the k points 
bool Update_KPoints(YsShellExt &Control_Mesh, std::unordered_map <int,YsVec3> &K_Points,std::unordered_map <YSHASHKEY,int> &K_Groups)
{

	Control_Mesh.EnableSearch(); //Enable search for the Control_Mesh vertex
	double Tolerance = 0.0001;
	int noMoved = 0; //Initialize the no of points that moved significantly to zero

	//For all the k points
	for (auto &k : K_Points)
	{

		auto grpID = k.first; //Get the group id of the point
		auto oldPos = k.second; //get the position of k point
		YsVec3 newPos(0.0,0.0,0.0);
		int count = 0; //Initialize count to zero

		//For all the vertex in the map
		for (auto &v : K_Groups)
		{
			//If vertex has same grp as the point, the execute below
			if (v.second == grpID)
			{				
				auto vtHd = Control_Mesh.FindVertex(v.first); //Get the vertex handle
				auto vtPos = Control_Mesh.GetVertexPosition(vtHd); //Get the vertex position
				newPos += vtPos; //update the newPos;
				count++;
			}
		}
		if (count != 0)
		{
			newPos = newPos/count; //Get the new Position of the point (which the centois of the vertex that belong to the same group)
			double movedDist  = (newPos - oldPos).GetLength(); //Calculate the move distance of the polygon
			if (movedDist >= Tolerance)
			{
				noMoved++;
			}
			K_Points.find(grpID)->second = newPos; //update the k points
		}
		
	}

	if (noMoved == 0) //If none of the points moved significantly
	{
		return true;
	}

	return false;

}

//This function colors all the polygon before biginning the shape deformation mode
void ColorAllPolygons(YsShellExt &mesh, YsVec3 plColor)
{
	for (auto plHd = mesh.NullPolygon(); true ==  mesh.MoveToNextPolygon(plHd);)
	{
		mesh.SetPolygonColor(plHd,YsColor(plColor.xf(),plColor.yf(),plColor.zf(),1)); //Set the polygon color
	}
}