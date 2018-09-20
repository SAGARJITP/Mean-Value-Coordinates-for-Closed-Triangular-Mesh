#ifndef GLUTIL_IS_INCLUDED
#define GLUTIL_IS_INCLUDED

#include <ysclass.h>

YsVec2i ViewPortToWindow(int winWid,int winHei,const YsVec3 &vp);
YsVec3 WindowToViewPort(int winWid,int winHei,int x,int y);
YsMatrix4x4 MakePerspective(const double fovy,const double aspect,const double nearz,const double farz);
YsMatrix4x4 MakeOrthogonal(const double left,const double right,const double bottom,const double top,const double nearz,const double farz);

#endif
