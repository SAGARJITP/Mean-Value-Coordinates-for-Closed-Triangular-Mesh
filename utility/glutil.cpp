#include "glutil.h"

YsVec2i ViewPortToWindow(int winWid,int winHei,const YsVec3 &vp)
{
	double s=(vp.x()+1.0)/2.0;
	double t=(1.0-vp.y())/2.0;
	double x=(double)winWid*s;
	double y=(double)winHei*t;
	return YsVec2i((int)x,(int)y);
}

YsVec3 WindowToViewPort(int winWid,int winHei,int x,int y)
{
	double vx=2.0*((double)x/(double)winWid)-1.0;
	double vy=1.0-2.0*((double)y/(double)winHei);
	return YsVec3(vx,vy,0.0);
}

YsMatrix4x4 MakePerspective(const double fovy,const double aspect,const double nearz,const double farz)
{
	float mat[16];

	// Based on the formula listed in www.opengl.org
	const float f=(float)(1.0/tan(fovy/2.0));
	mat[0]=(float)(f/aspect); mat[4]=0;        mat[ 8]=0;                                  mat[12]=0;
	mat[1]=0;                 mat[5]=(float)f; mat[ 9]=0;                                  mat[13]=0;
	mat[2]=0;                 mat[6]=0;        mat[10]=(float)((farz+nearz)/(nearz-farz)); mat[14]=(float)((2.0*farz*nearz)/(nearz-farz));
	mat[3]=0;                 mat[7]=0;        mat[11]=-1;                                 mat[15]=0;

	YsMatrix4x4 tfm;
	tfm.CreateFromOpenGlCompatibleMatrix(mat);
	return tfm;
}

YsMatrix4x4 MakeOrthogonal(
    const double left,const double right,const double bottom,const double top,const double nearz,const double farz)
{
	float mat[16];

	// Based on the formula listed in www.opengl.org
	const double tx=-(right+left)/(right-left);
	const double ty=-(top+bottom)/(top-bottom);
	const double tz=-(farz+nearz)/(farz-nearz);
	mat[0]=(float)(2.0/(right-left)); mat[4]=0;                         mat[ 8]=0;                          mat[12]=(float)tx;
	mat[1]=0;                         mat[5]=(float)(2.0/(top-bottom)); mat[ 9]=0;                          mat[13]=(float)ty;
	mat[2]=0;                         mat[6]=0;                         mat[10]=(float)(-2.0/(farz-nearz)); mat[14]=(float)tz;
	mat[3]=0;                         mat[7]=0;                         mat[11]=0;                          mat[15]=1;

	YsMatrix4x4 tfm;
	tfm.CreateFromOpenGlCompatibleMatrix(mat);
	return tfm;
}
