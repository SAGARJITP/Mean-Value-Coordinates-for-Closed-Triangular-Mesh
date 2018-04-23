#include <vector>

#include <ysclass.h>

#include <fslazywindow.h>

#include "ysshellext.h"
#include "objloader.h"
#include "meanValueInterpolation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>


class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;

	YsMatrix4x4 Rc;
	double d;
	YsVec3 t;

	YsShellExt Model_Mesh; //The Model mesh
	YsShellExt Control_Mesh; //The  control mesh
	std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map; //The weights map for every vertices of Model_Mesh

	std::vector <float> vtx,nom,col; //For model mesh
	std::vector <float> vtx_control, nom_control, col_control; //For Control Mesh
	YsVec3 bbx[2];

	
	static void AddColor(std::vector <float> &col,float r,float g,float b,float a);
	static void AddVertex(std::vector <float> &vtx,float x,float y,float z);
	static void AddNormal(std::vector <float> &nom,float x,float y,float z);

	void RemakeVertexArray(void);

public:
	FsLazyWindowApplication();
	virtual void BeforeEverything(int argc,char *argv[]);
	virtual void GetOpenWindowOption(FsOpenWindowOption &OPT) const;
	virtual void Initialize(int argc,char *argv[]);
	virtual void Interval(void);
	virtual void BeforeTerminate(void);
	virtual void Draw(void);
	virtual bool UserWantToCloseProgram(void);
	virtual bool MustTerminate(void) const;
	virtual long long int GetMinimumSleepPerInterval(void) const;
	virtual bool NeedRedraw(void) const;
};

/* static */ void FsLazyWindowApplication::AddColor(std::vector <float> &col,float r,float g,float b,float a)
{
	col.push_back(r);
	col.push_back(g);
	col.push_back(b);
	col.push_back(a);
}
/* static */ void FsLazyWindowApplication::AddVertex(std::vector <float> &vtx,float x,float y,float z)
{
	vtx.push_back(x);
	vtx.push_back(y);
	vtx.push_back(z);
}
/* static */ void FsLazyWindowApplication::AddNormal(std::vector <float> &nom,float x,float y,float z)
{
	nom.push_back(x);
	nom.push_back(y);
	nom.push_back(z);
}

void FsLazyWindowApplication::RemakeVertexArray(void)
{
	vtx.clear();
	col.clear();
	nom.clear();

	//Model Mesh
	for(auto plHd=Model_Mesh.NullPolygon(); true==Model_Mesh.MoveToNextPolygon(plHd); )
	{
		auto plVtHd=Model_Mesh.GetPolygonVertex(plHd);
		auto plCol=Model_Mesh.GetColor(plHd);
		auto plNom=Model_Mesh.GetNormal(plHd);

		// Let's assume every polygon is a triangle for now.

		for(int i=0; i<plVtHd.size(); ++i)
		{
			auto vtPos=Model_Mesh.GetVertexPosition(plVtHd[i]);
			vtx.push_back(vtPos.xf());
			vtx.push_back(vtPos.yf());
			vtx.push_back(vtPos.zf());
			nom.push_back(plNom.xf());
			nom.push_back(plNom.yf());
			nom.push_back(plNom.zf());
			col.push_back(plCol.Rf());
			col.push_back(plCol.Gf());
			col.push_back(plCol.Bf());
			col.push_back(plCol.Af());
		}	
	}

	// Control mesh
	for (auto plHd = Control_Mesh.NullPolygon(); true == Control_Mesh.MoveToNextPolygon(plHd); )
	{
		auto plVtHd = Control_Mesh.GetPolygonVertex(plHd);
		auto plCol = Control_Mesh.GetColor(plHd);
		auto plNom = Control_Mesh.GetNormal(plHd);

		// Let's assume every polygon is a triangle for now.
		if (3 == plVtHd.size())
		{
			for (int i = 0; i<3; ++i)
			{
				auto vtPos = Control_Mesh.GetVertexPosition(plVtHd[i]);
				vtx_control.push_back(vtPos.xf());
				vtx_control.push_back(vtPos.yf());
				vtx_control.push_back(vtPos.zf());
				nom_control.push_back(plNom.xf());
				nom_control.push_back(plNom.yf());
				nom_control.push_back(plNom.zf());
				col_control.push_back(plCol.Rf());
				col_control.push_back(plCol.Gf());
				col_control.push_back(plCol.Bf());
				col_control.push_back(plCol.Af());
			}
		}
	}



}

FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;
	d=35;
	t=YsVec3::Origin();
}

/* virtual */ void FsLazyWindowApplication::BeforeEverything(int argc,char *argv[])
{
}
/* virtual */ void FsLazyWindowApplication::GetOpenWindowOption(FsOpenWindowOption &opt) const
{
	opt.x0=0;
	opt.y0=0;
	opt.wid=1200;
	opt.hei=800;
}
/* virtual */ void FsLazyWindowApplication::Initialize(int argc,char *argv[])
{
	//if (2<=argc && mesh.LoadBinStl(argv[1]))
	if(3<=argc)
	{
		
		LoadObjFile(Model_Mesh,argv[1]); //load the  model mesh
		LoadObjFile(Control_Mesh,argv[2]); //Load  the control  mesh


		Weights_Map = GetMeanValueCoordinates(Model_Mesh,Control_Mesh); //Calculate the weights

		
		RemakeVertexArray();
		Model_Mesh.GetBoundingBox(bbx[0],bbx[1]);
		
		t=(bbx[0]+bbx[1])/2.0;
		d=(bbx[1]-bbx[0]).GetLength()*1.2;

		//printf("Target %s\n",t.Txt());
		//printf("Diagonal %lf\n",d);
		

	}
}
/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		SetMustTerminate(true);
	}

	if(FsGetKeyState(FSKEY_LEFT))
	{
		Rc.RotateXZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_RIGHT))
	{
		Rc.RotateXZ(-YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_UP))
	{
		Rc.RotateYZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_DOWN))
	{
		Rc.RotateYZ(-YsPi/60.0);
	}
	if (FsGetKeyState(FSKEY_PLUS))
	{
		d = d - 0.5;
	}
	if (FsGetKeyState(FSKEY_MINUS))
	{
		d = d + 0.5;
	}
	//Test the MeanValue Interpolation................................................
	if (FsGetKeyState(FSKEY_D))
	{

		// Translating each vertex by some amount in x-direction
		for (auto vtxHd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(vtxHd); )
		{
			Control_Mesh.SetVertexPosition(vtxHd, Control_Mesh.GetVertexPosition(vtxHd) + YsVec3(1.0,0.0,0.0));
		}

		//Translating corresponding Model_Mesh using Mean Value Coordinates
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

			//printf("Old: %lf %lf %lf\n", Model_Mesh.GetVertexPosition(MM_Vertex_Hd).xf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).yf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).zf());
			YsVec3 NewPos = TotalF/TotalW; //Calculate new position for Model Mesh vertex
			Model_Mesh.SetVertexPosition(MM_Vertex_Hd,NewPos); //Set new position for the Model Mesh vertex
			//printf("New: %lf %lf %lf\n", Model_Mesh.GetVertexPosition(MM_Vertex_Hd).xf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).yf(), Model_Mesh.GetVertexPosition(MM_Vertex_Hd).zf());

			i++;
		}

		RemakeVertexArray();

		printf("Complete\n");

	}


	needRedraw=true;
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
	needRedraw=false;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto aspect=(double)wid/(double)hei;
	glViewport(0,0,wid,hei);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,aspect,d/10.0,d*2.0);

	YsMatrix4x4 globalToCamera=Rc;
	globalToCamera.Invert();

	YsMatrix4x4 modelView;  // need #include ysclass.h
	modelView.Translate(0,0,-d);
	modelView*=globalToCamera;
	modelView.Translate(-t);

	GLfloat modelViewGl[16];
	modelView.GetOpenGlCompatibleMatrix(modelViewGl);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	

	GLfloat lightDir[]={0.0f,1.0f/(float)sqrt(2.0f),1.0f/(float)sqrt(2.0f),0.0f};
	glLightfv(GL_LIGHT0,GL_POSITION,lightDir);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMultMatrixf(modelViewGl);

	
	//Draw Model Mesh
	/*
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4,GL_FLOAT,0,col.data());
	glNormalPointer(GL_FLOAT,0,nom.data());
	glVertexPointer(3,GL_FLOAT,0,vtx.data());
	glDrawArrays(GL_TRIANGLES,0,vtx.size()/3);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	*/



	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4,GL_FLOAT,0,col_control.data());
	glNormalPointer(GL_FLOAT,0,nom_control.data());
	glVertexPointer(3,GL_FLOAT,0,vtx_control.data());
	glDrawArrays(GL_TRIANGLES,0,vtx_control.size()/3);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	// Draw Control Mesh
	for (int idx = 0; idx < vtx_control.size()/3; idx += 3) {
		glBegin(GL_LINE_LOOP);
		glVertex3d(vtx_control[3*idx], vtx_control[3*idx + 1], vtx_control[3*idx + 2]);
		glVertex3d(vtx_control[3 * (idx + 1)], vtx_control[3 * (idx + 1) + 1], vtx_control[3 * (idx + 1) + 2]);
		glVertex3d(vtx_control[3 * (idx + 2)], vtx_control[3 * (idx + 2) + 1], vtx_control[3 * (idx + 2)+ 2]);
		glEnd();
	}

	

	FsSwapBuffers();
}
/* virtual */ bool FsLazyWindowApplication::UserWantToCloseProgram(void)
{
	return true; // Returning true will just close the program.
}
/* virtual */ bool FsLazyWindowApplication::MustTerminate(void) const
{
	return FsLazyWindowApplicationBase::MustTerminate();
}
/* virtual */ long long int FsLazyWindowApplication::GetMinimumSleepPerInterval(void) const
{
	return 10;
}
/* virtual */ void FsLazyWindowApplication::BeforeTerminate(void)
{
}
/* virtual */ bool FsLazyWindowApplication::NeedRedraw(void) const
{
	return needRedraw;
}


static FsLazyWindowApplication *appPtr=nullptr;

/* static */ FsLazyWindowApplicationBase *FsLazyWindowApplicationBase::GetApplication(void)
{
	if(nullptr==appPtr)
	{
		appPtr=new FsLazyWindowApplication;
	}
	return appPtr;
}
