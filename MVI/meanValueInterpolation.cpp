#include "meanValueInterpolation.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//Compute the determinant of the 
float Determinant(YsVec3 &u0, YsVec3 &u1, YsVec3 &u2)
{
	float det = u0.xf()*( u1.yf()*u2.zf() - u2.yf()*u1.zf() ) - u0.yf()*(u1.xf()*u2.zf() - u2.xf()*u1.zf()) + u0.zf()*(u1.xf()*u2.yf() - u2.xf()*u1.yf());
	return det;
}

//This function takes the Model_Mesh and Control_Mesh and generates the mean values coordinates for each vertex in Model_Mesh
std::vector <std::unordered_map <YSHASHKEY,float>> GetMeanValueCoordinates(YsShellExt &Model_Mesh, YsShellExt &Control_Mesh)
{

	auto MM_Vertex_Hd= Model_Mesh.NullVertex();
	Model_Mesh.MoveToNextVertex(MM_Vertex_Hd); //Get the first vertex handle of modal mesh

	std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map; //Vector of unordered_map 


	//Loop through every vertex of the Model_Mesh
	for (auto MM_Vertex_Hd = Model_Mesh.NullVertex(); true == Model_Mesh.MoveToNextVertex(MM_Vertex_Hd);)
	{

		YsVec3 MM_Vertex_Pos = Model_Mesh.GetVertexPosition(MM_Vertex_Hd);
		auto map = GetVertexMeanValues(MM_Vertex_Pos, Control_Mesh);
		Weights_Map.push_back(map);

	}
	
	return Weights_Map;

}

//This function gives the weights for a particular vertex of Model_Mesh corresponding to the vertices of the Control_Mesh
std::unordered_map <YSHASHKEY,float> GetVertexMeanValues(const YsVec3 MM_Vertex_Pos, const YsShellExt &Control_Mesh)
{

	
	float Tolerance = 0.01; //Set the tolerance value
	
	std::unordered_map <YSHASHKEY,float> Weights; //unordered map that stores the weight for M_Vertex corresponding to each vertex of the Control_Mesh

	//Initialize all the weights to zero
	for (auto CM_Vertex_Hd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(CM_Vertex_Hd);)
	{
		Weights.insert({Control_Mesh.GetSearchKey(CM_Vertex_Hd),0});
	}

	//If MM_Veretx is very close to any node of the Control of the Control_Mesh
	for (auto CM_Vertex_Hd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(CM_Vertex_Hd);)
	{
		YsVec3 p = Control_Mesh.GetVertexPosition(CM_Vertex_Hd);
		float d = (p - MM_Vertex_Pos).GetLength();
		if (d  <= Tolerance)
		{
			Weights.find(Control_Mesh.GetSearchKey(CM_Vertex_Hd))->second = 1;
			return Weights;
		}	

	}

	// If above condition is false...........................................
	//Looping through all triangular mesh in the Control_Mesh. Let each triangle be T
	for(auto CM_plHd = Control_Mesh.NullPolygon(); true == Control_Mesh.MoveToNextPolygon(CM_plHd);)
	{

		float w0, w1, w2;

		//Get the vertex handles for the triangle T
		auto Tri_VtxHd = Control_Mesh.GetPolygonVertex(CM_plHd); //Get the vertex handles of the vertices of the triangle

		YsVec3 p0 = Control_Mesh.GetVertexPosition(Tri_VtxHd[0]); //Get the vertex position of 1st vertex of triangle
		YsVec3 p1 = Control_Mesh.GetVertexPosition(Tri_VtxHd[1]); // Get the vertex position of 2nd vertex of triangle
		YsVec3 p2 = Control_Mesh.GetVertexPosition(Tri_VtxHd[2]); //Get the vertex position of 3rd vertex of triangle

		float d0 = (p0 - MM_Vertex_Pos).GetLength(); //Get the distance of MM_Vertex from 1st vertex of triangle
		float d1 = (p1 - MM_Vertex_Pos).GetLength(); //Get the distance of MM_Vertex from 2nd vertex of triangle
		float d2 = (p2 - MM_Vertex_Pos).GetLength(); //Get the distance of MM_Vertex from 3rd vertex of triangle


		//Get the unit vectors from the vertices of triangles to the MM_Vertex
		YsVec3 u0 = (p0 - MM_Vertex_Pos)/d0;
		YsVec3 u1 = (p1 - MM_Vertex_Pos)/d1;
		YsVec3 u2 = (p2 - MM_Vertex_Pos)/d2;


		//Calculate the h parameter for the triangle of the Control_Mesh
		float l0 = (u1 - u2).GetLength(), l1 = (u0 - u2).GetLength(), l2 = (u0 - u1).GetLength();
		float theta_0 = 2*asin(l0/2), theta_1 = 2*asin(l1/2), theta_2 = 2*asin(l2/2);
		float h = (theta_0 + theta_1 + theta_2)/2;


		//if h is close to pi, then the MM_VErtex lies on Trianle T. Hence use 2D  barycentric coordinates
		if ((YsPi - h) <= Tolerance)
		{

			//Find more efficient way for this......................................................................
			//Initialize all the weights to zero
			for (auto CM_Vertex_Hd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(CM_Vertex_Hd);)
			{
				//Weights.insert({Control_Mesh.GetSearchKey(CM_Vertex_Hd),0});
				Weights.find(Control_Mesh.GetSearchKey(CM_Vertex_Hd))->second = 0;		
			}
			//.........................................

			w0 = sin(theta_0)*l1*l2;
			w1 = sin(theta_1)*l0*l2; 
			w2 = sin(theta_2)*l0*l1;
			Weights.find(Control_Mesh.GetSearchKey(Tri_VtxHd[0]))->second = w0; //Only the vertices of the triangle will  come into picture here for the weights
			Weights.find(Control_Mesh.GetSearchKey(Tri_VtxHd[1]))->second = w1; //weights due to vertices of other triangles will be zero
			Weights.find(Control_Mesh.GetSearchKey(Tri_VtxHd[2]))->second = w2;
			return Weights;
			break; //Return 
		}


		//If MM_Vertex does not lie in the plane of the vertex. then we calculate 3d mean value coordinates usinfg spherical triangles
		float c0 = (2*sin(h)*sin(h - theta_0))/(sin(theta_1)*sin(theta_2)) - 1;
		float c1 = (2*sin(h)*sin(h - theta_1))/(sin(theta_0)*sin(theta_2)) - 1;
		float c2 = (2*sin(h)*sin(h - theta_2))/(sin(theta_0)*sin(theta_1)) - 1;

		float det = Determinant(u0,u1,u2);
		float sign  = det/abs(det);

		float s0 = sign*(sqrt(1 - c0*c0)), s1 = sign*(sqrt(1 - c1*c1)), s2 = sign*(sqrt(1 - c2*c2));


		//If the MM_Vertex lies outside T in the same plane, then ignore
		if (abs(s0) <=  Tolerance || abs(s1) <= Tolerance || abs(s2) <= Tolerance)
		{
			continue;
		}

		//If MM_Vertex lies inside the Control_Mesh
		w0 = (theta_0 - c1*theta_2 - c2*theta_1) / (2*d0*sin(theta_2)*sqrt(1 - c1*c1));
		w1 = (theta_1 - c0*theta_2 - c2*theta_0) / (2*d1*sin(theta_0)*sqrt(1 - c2*c2));
		w2 = (theta_2 - c1*theta_0 - c0*theta_1) / (2*d2*sin(theta_1)*sqrt(1 - c0*c0));

		Weights.find(Control_Mesh.GetSearchKey(Tri_VtxHd[0]))->second += w0; //update the weights . Add to the weights
		Weights.find(Control_Mesh.GetSearchKey(Tri_VtxHd[1]))->second += w1;
		Weights.find(Control_Mesh.GetSearchKey(Tri_VtxHd[2]))->second += w2;

	}

	return Weights;

}