#include <vector>

#include <ysclass.h>
#include <glutil.h>
#include "meshDeform.h"

#include <fslazywindow.h>

#include "ysshellext.h"
#include "objloader.h"
#include "meanValueInterpolation.h"

#include <unordered_set>

#include <stdio.h>
#include <stdlib.h>
#include <string>

#define INF_D (std::numeric_limits<float>::infinity())


class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;

	YsMatrix4x4 Rc;
	double d;
	YsVec3 t;
	std::vector <YsShellExt::VertexHandle> PickedVertices; //vectorof vertices picked by the mouse to move
	bool moveVertex; //flag to indicate whether to move vertices of control mesh or not

	YsShellExt Model_Mesh; //The Model mesh
	YsShellExt Control_Mesh; //The control mesh
	std::vector <std::unordered_map <YSHASHKEY,float>> Weights_Map; //The weights map for every vertices of Model_Mesh

	std::vector <float> vtx,nom,col; //For model mesh
	std::vector <float> vtx_control, nom_control, col_control; //For Control Mesh
	YsVec3 bbx[2];//bounding box of the mesh

	
	static void AddColor(std::vector <float> &col,float r,float g,float b,float a);
	static void AddVertex(std::vector <float> &vtx,float x,float y,float z);
	static void AddNormal(std::vector <float> &nom,float x,float y,float z);


	YsMatrix4x4 GetProjection(void) const;
	YsMatrix4x4 GetModelView(void) const;
	YsShellExt::PolygonHandle PickedPlHd(int mx,int my) const;
	YsShellExt::VertexHandle PickedVtHd(int mx,int my,int pixRange) const;

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

	vtx_control.clear();
	col_control.clear();
	nom_control.clear();

	//Model Mesh
	for(auto plHd=Model_Mesh.NullPolygon(); true==Model_Mesh.MoveToNextPolygon(plHd); )
	{
		auto plVtHd=Model_Mesh.GetPolygonVertex(plHd);
		auto plCol=Model_Mesh.GetColor(plHd);
		auto plNom=Model_Mesh.GetNormal(plHd);

		
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
			for (int i = 0; i<plVtHd.size(); ++i)
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

YsMatrix4x4 FsLazyWindowApplication::GetProjection(void) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto aspect=(double)wid/(double)hei;
	return MakePerspective(45.0,aspect,d/10.0,d*2.0);
}

YsMatrix4x4 FsLazyWindowApplication::GetModelView(void) const
{
	YsMatrix4x4 globalToCamera=Rc;
	globalToCamera.Invert();

	YsMatrix4x4 modelView;
	modelView.Translate(0,0,-d);
	modelView*=globalToCamera;
	modelView.Translate(-t);
	return modelView;
}

//Get the picked polygon handle of control mesh
YsShellExt::PolygonHandle FsLazyWindowApplication::PickedPlHd(int mx,int my) const
{
	YsVec3 mos[2];

	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto p=WindowToViewPort(wid,hei,mx,my);
	mos[0]=p;
	mos[1]=p;
	mos[0].SetZ(-1.0);
	mos[1].SetZ( 1.0);

	auto pers=GetProjection();
	pers.MulInverse(mos[0],mos[0],1.0);
	pers.MulInverse(mos[1],mos[1],1.0);

	auto modelView=GetModelView();
	modelView.MulInverse(mos[0],mos[0],1.0);
	modelView.MulInverse(mos[1],mos[1],1.0);

	double maxZ=0.0;
	YsShellExt::PolygonHandle plHd=nullptr,pickedPlHd=nullptr;
	while(true==Control_Mesh.MoveToNextPolygon(plHd))
	{
		auto plVtHd=Control_Mesh.GetPolygonVertex(plHd);
		std::vector <YsVec3> plVtPos;
		plVtPos.resize(plVtHd.size());
		for(int i=0; i<plVtHd.size(); ++i)
		{
			plVtPos[i]=Control_Mesh.GetVertexPosition(plVtHd[i]);
		}

		YsPlane pln;
		if(YSOK==pln.MakeBestFitPlane(plVtPos))
		{
			YsVec3 itsc;
			if(YSTRUE==pln.GetPenetration(itsc,mos[0],mos[1]))
			{
				auto side=YsCheckInsidePolygon3(itsc,plVtPos);
				if(YSINSIDE==side || YSBOUNDARY==side)
				{
					auto itscInView=modelView*itsc;
					if(nullptr==pickedPlHd || itscInView.z()>maxZ)
					{
						maxZ=itscInView.z();
						pickedPlHd=plHd;
					}
				}
			}
		}
	}

	return pickedPlHd;
}


//Get the vertex handle of the of the vertex of the control mesh picked by mouse
YsShellExt::VertexHandle FsLazyWindowApplication::PickedVtHd(int mx,int my,int pixRange) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);


	printf("distance from camera %lf\n", d);
	printf("mouse positions: %d %d\n",mx,my);


	//auto vp=WindowToViewPort(wid,hei,mx,my);


	auto projection=GetProjection();
	auto modelView=GetModelView();

	double pickedZ=0.0;

	
	auto pickedVtHd=Control_Mesh.NullVertex();
	for(auto vtHd=Control_Mesh.NullVertex(); true==Control_Mesh.MoveToNextVertex(vtHd); )
	{
		auto vtPos=Control_Mesh.GetVertexPosition(vtHd);
		vtPos=projection*modelView*vtPos;
		auto winPos=ViewPortToWindow(wid,hei,vtPos); //here ViewPort means the clip coordinates (i guess)
		

		//printf("%d winPos: %lf %lf %lf\n",Control_Mesh.GetSearchKey(vtHd), winPos.xf(),winPos.yf(),vtPos.z());

		int dx=(mx-winPos.x()),dy=(my-winPos.y());
		//double d = sqrt(dx*dx + dy*dy);
		if(-pixRange<=dx && dx<=pixRange && -pixRange<=dy && dy<=pixRange)
		{
			if(Control_Mesh.NullVertex()==pickedVtHd || vtPos.z()<pickedZ)
			{
				pickedVtHd=vtHd;
				pickedZ=vtPos.z();
			}
		}
	}
	


	/*
	YsShellExt::PolygonHandle pickedPolygon = PickedPlHd(mx,my);
	float distance = INF_D;
	auto vertices = Control_Mesh.GetPolygonVertex(pickedPolygon);
	YsShellExt::VertexHandle pickedVtHd = Control_Mesh.NullVertex();;
	for (auto vertex : vertices) 
	{
		auto vtPos = Control_Mesh.GetVertexPosition(vertex);
		vtPos = projection*modelView*vtPos;
		auto winPos = ViewPortToWindow(wid, hei, vtPos);
		int dx = (mx - winPos.x()), dy = (my - winPos.y());

		printf("%d distance: %lf\n",  Control_Mesh.GetSearchKey(vertex), sqrt(dx*dx + dy*dy));
		if (sqrt(dx*dx + dy*dy) < distance)
		{
			pickedVtHd = vertex;
			distance = sqrt(dx*dx + dy*dy);
		}
			
	}
	*/
	
	

	return pickedVtHd;

}

///////////////////////////////////////////////////////////////////
FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;
	//d=35;
	t=YsVec3::Origin();
	moveVertex = false;
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
		Control_Mesh.GetBoundingBox(bbx[0],bbx[1]);
		
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
		if (moveVertex)
		{
			MoveControlMesh(Control_Mesh,PickedVertices,YsVec3(-0.05,0.00,0.00));
			RemakeVertexArray();			
		}
		else
		{
			Rc.RotateXZ(YsPi/60.0);
		}
		
	}
	if(FsGetKeyState(FSKEY_RIGHT))
	{
		if (moveVertex)
		{
			MoveControlMesh(Control_Mesh,PickedVertices,YsVec3(0.05,0.00,0.00));
			RemakeVertexArray();
		}
		else
		{
			Rc.RotateXZ(-YsPi/60.0);
		}
		
	}
	if(FsGetKeyState(FSKEY_UP))
	{
		if (moveVertex)
		{
			MoveControlMesh(Control_Mesh,PickedVertices,YsVec3(0.00,0.05,0.00));
			RemakeVertexArray();
		}
		else
		{
			Rc.RotateYZ(YsPi/60.0);
		}
		
	}
	if(FsGetKeyState(FSKEY_DOWN))
	{
		if (moveVertex)
		{
			MoveControlMesh(Control_Mesh,PickedVertices,YsVec3(0.00,-0.05,0.00));
			RemakeVertexArray();
		}
		else
		{
			Rc.RotateYZ(-YsPi/60.0);
		}
		
	}
	if (FsGetKeyState(FSKEY_PLUS))
	{
		if (moveVertex)
		{
			MoveControlMesh(Control_Mesh,PickedVertices,YsVec3(0.00,0.00,0.05));
			RemakeVertexArray();
		}
		else
		{
			d -= 0.5;
		}
		
	}
	if (FsGetKeyState(FSKEY_MINUS))
	{
		if (moveVertex)
		{
			MoveControlMesh(Control_Mesh,PickedVertices,YsVec3(0.00,0.00,-0.05));
			RemakeVertexArray();
		}
		else
		{
			d += 0.5;
		}
		
	}
	if (FsGetKeyState(FSKEY_M)) //enable move vertices
	{
		PickedVertices.clear();
		moveVertex = true;
	}
	if (FsGetKeyState(FSKEY_S)) //disable move vertices
	{
		moveVertex = false;
	}

	if (FsGetKeyState(FSKEY_Q)) //scale up
	{
		ScaleUp(Control_Mesh);
		RemakeVertexArray();
	}
	if (FsGetKeyState(FSKEY_W)) //scale  down
	{
		ScaleDown(Control_Mesh);
		RemakeVertexArray();
	}
	
	//Move Model Mesh
	if (FsGetKeyState(FSKEY_D))
	{

		// Translating each vertex by some amount in x-direction
		//for (auto vtxHd = Control_Mesh.NullVertex(); true == Control_Mesh.MoveToNextVertex(vtxHd); )
		//{
		//	Control_Mesh.SetVertexPosition(vtxHd, Control_Mesh.GetVertexPosition(vtxHd) + YsVec3(0.2,0.0,0.0));
		//}

		MoveModelMesh(Control_Mesh, Model_Mesh,Weights_Map);
		RemakeVertexArray();
	}

	int lb,mb,rb,mx,my;
	auto evt=FsGetMouseEvent(lb,mb,rb,mx,my);
	if(evt==FSMOUSEEVENT_LBUTTONDOWN)
	{

		auto pickedVtHd=PickedVtHd(mx,my,30);
		if(nullptr!=pickedVtHd)
		{
			printf("%d \n", Control_Mesh.GetSearchKey(pickedVtHd));
			PickedVertices.push_back(pickedVtHd);
		}

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
	

	// Draw Control Mesh
	for (int idx = 0; idx < vtx_control.size()/3; idx += 3) {
		glBegin(GL_LINE_LOOP);
		glVertex3d(vtx_control[3*idx], vtx_control[3*idx + 1], vtx_control[3*idx + 2]);
		glVertex3d(vtx_control[3 * (idx + 1)], vtx_control[3 * (idx + 1) + 1], vtx_control[3 * (idx + 1) + 2]);
		glVertex3d(vtx_control[3 * (idx + 2)], vtx_control[3 * (idx + 2) + 1], vtx_control[3 * (idx + 2)+ 2]);
		glEnd();
	}


	//Draw Control Nodes
	/*
	glEnableClientState(GL_VERTEX_ARRAY);
	glPointSize(4);
	glVertexPointer(3,GL_FLOAT,0,vtx_control.data());
	glDrawArrays(GL_POINTS,0,vtx.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	*/
	

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
