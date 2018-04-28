#include "objloader.h"
#include "ysshellext.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <vector>

#include <ysclass.h>





//This function is used to read a line and save all the words in the line separated by a space,tab or comma
int ParseString(int &nWord, int wordTop[], int wordLength[], char str[])
{
    int i,state;

    state=0;
    nWord=0;
    for(i=0; str[i]!=0; i++)
    {
        switch(state)
        {
        case 0:
            if(str[i]!=' ' && 
               str[i]!='\t' &&
               str[i]!=',')
            {
                state=1;
                wordTop[nWord]=i;
                wordLength[nWord]=1;
                nWord++;
            }
            break;
        case 1:
            if(str[i]!=' ' &&
               str[i]!='\t' &&
               str[i]!=',')
            {
                wordLength[nWord-1]++;
            }
            else
            {
                state=0;
            }
            break;
        }
    }

    return nWord;
}

//Safely copies the string from source to destination
void SafeStrCpy(char dst[],char src[],int nLetters,int nLimit)
{
    int i,stopper;
    if(nLetters<nLimit)
    {
        stopper=nLetters;
    }
    else
    {
        stopper=nLimit;
    }

    for(i=0; i<stopper; i++)
    {
        dst[i]=src[i];
    }
    dst[stopper]=0;
}

//Same as Fgets buts removes the control character at the end
char *YsFgets(char buf[],unsigned long long int maxSize,FILE *fp)
{

    buf[0]=0;
    if(nullptr!=fgets(buf,maxSize,fp))
    {
        int ptr=0;
        for(ptr=0; 0!=buf[ptr]; ++ptr)
        {
        }
        for(ptr=ptr-1; 0<=ptr && 0==isprint(buf[ptr]); --ptr)
        {
            buf[ptr]=0;
        }
        return buf;
    }
    return nullptr;
}

//Remove the part after '/' and returns the  part before
int GetVertexId(char vtxIdx[])
{
	char id[256];
	int vid;

	int i = 0;
	while(vtxIdx[i] != 0 && vtxIdx[i] != '/')
	{
		id[i] = vtxIdx[i];
		i++;
	}

	vid = atoi(id);

	return vid;

}

//This function reads the OBJ file and stores the mesh data in mesh 
bool LoadObjFile(YsShellExt &mesh, const char fn[])
{


	FILE *fp = fopen(fn, "rb"); //open the OBJ file

    
	if (nullptr!=fp)
	{

        printf("Loading: %s\n",fn);
		std::vector <YsVec3> Vtx; //Define A vector of vertices
		std::vector <YsShellExt::VertexHandle> vtHdArray; //Define a vector of vertex handle

		char *line = new char[256]; //This variable store the data in a line in the file


		while (YsFgets(line,255,fp) != nullptr) //Maximum limit on character in a line is 255. Change if needed
		{

			char word[256];
    		int nWord,wordTop[16],wordLength[16]; //Maximum limit on words in a line is 16. Change if needed



    		ParseString(nWord,wordTop,wordLength,line);
    		SafeStrCpy(word,line,wordLength[0],255); //Get the first command of the line //Change the limit from 255 if needed
    		

    		//If the first command of the line is 'v' , save the vertex in the polygonal mesh
    		if (!strcmp(word,"v"))
    		{
    			
    			char *xstr = new char[256];
    			char *ystr = new char[256];
    			char *zstr = new char[256]; //Dynamically allocate the memory
    			float vx,vy,vz; //x,y,z coordinates of the vertex

		        SafeStrCpy(xstr,line+wordTop[1],wordLength[1],255);
		        SafeStrCpy(ystr,line+wordTop[2],wordLength[2],255);
		        SafeStrCpy(zstr,line+wordTop[3],wordLength[3],255);	

		        vx = atof(xstr); //Get the x coordinate of the vertex
		        vy = atof(ystr); //Get the y coordinate of the vertex
		        vz = atof(zstr); //Get the z coordinate of the vertex

		        vtHdArray.push_back(mesh.AddVertex(YsVec3(vx,vy,vz))); //Add the vertex to the mesh and create the vertex handle foe that

		        delete [] xstr, ystr, zstr; //Release the memory
		        
    		}	

            	

    		//If the first command of the line is 'f', then add the polygon to the mesh data structure   
    		if (!strcmp(word,"f"))
    		{

    			std::vector <YsShellExt::VertexHandle> plHd;
    			
    			int numVtxPlg = nWord - 1; //Get the number of vertcices that make up the polygon

    			//printf("f ");
    			for (int i = 1; i <= numVtxPlg;  i++)
    			{
    				char *vtxIdx = new char[256]; //This variable is used to store the vertexx id that make up  the polygon
    				SafeStrCpy(vtxIdx,line+wordTop[i],wordLength[i],255);
    				int idx = GetVertexId(vtxIdx);
    				//printf("%d ",idx);
    				plHd.push_back(vtHdArray[idx-1]);
    				delete [] vtxIdx;		        	
    			}
    			//printf("\n");

    			mesh.AddPolygon(plHd);
    			 			
    		}	

	      

		}

		delete [] line;



		//Set color and normal for polygon
		for(auto plHd=mesh.NullPolygon(); true==mesh.MoveToNextPolygon(plHd); )
		{

			YsVec3 plgNom;
		

			auto plVtHd=mesh.GetPolygonVertex(plHd);
			//Get all three vertices
			auto vtPos1 = mesh.GetVertexPosition(plVtHd[0]);
			auto vtPos2 = mesh.GetVertexPosition(plVtHd[1]);
			auto vtPos3 = mesh.GetVertexPosition(plVtHd[2]);

			YsVec3 vec1(vtPos2.xf() - vtPos1.xf(), vtPos2.yf() - vtPos1.yf(), vtPos2.zf() - vtPos1.zf());
			YsVec3 vec2(vtPos3.xf() - vtPos1.xf(), vtPos3.yf() - vtPos1.yf(), vtPos3.zf() - vtPos1.zf());
			
			plgNom = YsUnitVector(vec1^vec2);

			mesh.SetPolygonNormal(plHd, plgNom); //Set Polygon Color
			mesh.SetPolygonColor(plHd,YsColor(0,255,255,1)); //Set Polygon Color
		}
		
		
		printf("NumVertex = %d\n",  mesh.GetNumVertex());
		printf("NumPolygon = %d\n", mesh.GetNumPolygon());
	

		fclose(fp);
		return true;
	}
    else
    {
        printf("Could not load the file\n");
    }

	

	return false;
}

//Sve the mesh in OBJ format
bool SaveObj (YsShellExt &mesh, const char fn[])
{
    FILE *fp = fopen(fn, "wb");


    mesh.EnableSearch();

    if (nullptr != fp)
    {

        int numVertex = mesh.GetNumVertex();


        //Print the vertex data
        for (int i = 1; i <= numVertex; i++)
        {
            auto vtHd = mesh.FindVertex(i); //Get the handle of the vertex
            auto vtPos = mesh.GetVertexPosition(vtHd);
            fprintf(fp, "v %lf %lf %lf\n", vtPos.xf(),vtPos.yf(),vtPos.zf());
        }
       // fprintf(fp,"\n");


        //Print the face data
        for (auto plHd = mesh.NullPolygon(); true == mesh.MoveToNextPolygon(plHd);)
        {
            auto plVertex = mesh.GetPolygonVertex(plHd); // Get all verrtex of the polygon

            int v1 = mesh.GetSearchKey(plVertex[0]); //Get the key for the first vertex of the triangle 
            int v2 = mesh.GetSearchKey(plVertex[1]); //Get the key for the second vertex of the triangle
            int v3 = mesh.GetSearchKey(plVertex[2]); //Get the  key for the third vertexx of the triangle
            fprintf(fp, "f %d %d %d\n", v1,v2,v3);

        }

        return true;
    }

    return false;

}


//Save polygonal mesh data to binary stl file
bool SaveBinStl(const YsShellExt &mesh, const char fn[]) 
{

    FILE *bin_fp = fopen(fn,"wb");

    if (nullptr!=bin_fp)
    {

        //Write comments
        unsigned char comments[80] = {0};
        fwrite(comments,1,80,bin_fp); //First 80 bytes are comments

        //Write num of polygon data
        int NumPolygon = mesh.GetNumPolygon();
        fwrite((unsigned char*)&NumPolygon,1,4,bin_fp); //next 4 bytes are the number of polygons

        int count = 0;
        for(auto plHd=mesh.NullPolygon(); true==mesh.MoveToNextPolygon(plHd); )
        {

            auto plVtHd=mesh.GetPolygonVertex(plHd);
            auto plNom=mesh.GetNormal(plHd);

            //Write Normal data
            float nom_x = plNom.xf(), nom_y =  plNom.yf(), nom_z =  plNom.zf();
            fwrite((unsigned char*)&nom_x,1,4,bin_fp); //x coord for normal
            fwrite((unsigned char*)&nom_y,1,4,bin_fp); //y coord for normal
            fwrite((unsigned char*)&nom_z,1,4,bin_fp); //z coord for normal

            //Write vertex data
            for (int i = 0; i < 3; i++)
            {
                auto vtPos=mesh.GetVertexPosition(plVtHd[i]);
                float v_x = vtPos.xf() , v_y =vtPos.yf(), v_z =vtPos.zf() ;
                fwrite((unsigned char*)&v_x,1,4,bin_fp); //x coordd for vertex
                fwrite((unsigned char*)&v_y,1,4,bin_fp); //y coord for vertex
                fwrite((unsigned char*)&v_z,1,4,bin_fp); //z coord for vertex
            }

            //Write element id data
            char vid[2] = {count};
            fwrite(vid,1,2,bin_fp);

            count++;
        }

        fclose(bin_fp);
        return true;
    }
    return false;
}

