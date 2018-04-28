
#ifndef OBJLOADER_IS_INCLUDED
#define OBJLOADER_IS_INCLUDED

#include "ysshellext.h"




int ParseString(int &nWord, int wordTop[], int wordLength[], char str[]);

void SafeStrCpy(char dst[],char src[],int nLetters,int nLimit);

char *YsFgets(char buf[],unsigned long long int maxSize,FILE *fp);

int GetVertexId(char vtxIdx[]);

bool LoadObjFile(YsShellExt &mesh, const char fn[]);

//Save the mesh in OBJ format
bool SaveObj (YsShellExt &mesh, const char fn[]);

//Save polygonal mesh data to binary stl file
bool SaveBinStl(const YsShellExt &mesh, const char fn[]) ;




#endif