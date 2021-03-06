#include "lightfield.h"
#include <iostream>

using namespace std;

Plane::Plane(float z, float width, float height, int xRes, int yRes) {
	
/*	  ________ xRes _______
	/_______________________\
   /|_|_|_|_|_|_|_|_|_|_|_|_|
  | |_|						|
yRes|_|		z (camera)		| height (camera)
  | |_|						|
   \|_|_____________________|
		  width (camera)
*/
	this->z = z;
	this->width = width;
	this->height = height;
	this->xRes = xRes;
	this->yRes = yRes;
}

bool Plane::Intersect(Ray& ray, int* coords) {
	
	float t = (z - ray.o.z) / ray.d.z;
	float xInt = ray.o.x + t*ray.d.x;
	float yInt = ray.o.y + t*ray.d.y;
	
	coords[0] = (xInt*xRes)/width + xRes/2;
	coords[1] = (yInt*yRes)/height + yRes/2;
	if (coords[0] >= 0 && coords[0] < xRes && coords[1] >= 0 && coords[1] < yRes)
		return true;
		
	return false;
}

UVPlane::UVPlane(float z, float width, float height, int xRes, int yRes) : Plane(z, width, height, xRes, yRes) {}

STPlane::STPlane(float z, float width, float height, int xRes, int yRes) : Plane(z, width, height, xRes, yRes) {
	
	r = new short*[xRes];
	g = new short*[xRes];
	b = new short*[xRes];
	for (int i = 0; i < xRes; i++) {
		r[i] = new short[yRes];
		g[i] = new short[yRes];
		b[i] = new short[yRes];
		for (int j = 0; j < yRes; j++) {
			r[i][j] = g[i][j] = b[i][j] = 0.f;
		}
	}
}

LightField::LightField(float zClose, float widthClose, float heightClose, int xResClose, int yResClose,
 		   			   float zFar, float widthFar, float heightFar, int xResFar, int yResFar) {
						
	uvPlane = new UVPlane(zClose, widthClose, heightClose, xResClose, yResClose);					
	stPlanes = new STPlane*[xResClose*yResClose];
	for (int i = 0; i < xResClose*yResClose; i++) {
		stPlanes[i] = new STPlane(zFar, widthFar, heightFar, xResFar, yResFar);
	}
}

bool LightField::Intersect(Ray& ray, float* rgb) {
		
	int uvCoords[2];
	if (uvPlane->Intersect(ray, uvCoords)) {
		int count = 0;
		rgb[0] = rgb[1] = rgb[2] = 0;
		for (int i = 0; i <= 0; i++) {
			for (int j = 0; j <= 0; j++) {
				int x = uvCoords[0];
				int y = uvCoords[1];
				int ind = (x+i) + uvPlane->xRes*(y+j);
				if (ind >= 0 && ind < uvPlane->xRes * uvPlane->yRes) {
					STPlane *stPlane = stPlanes[ind];
					int stCoords[2];
					if (stPlane->Intersect(ray, stCoords)) {
						rgb[0] += (float)stPlane->r[stCoords[0]][stCoords[1]];
						rgb[1] += (float)stPlane->g[stCoords[0]][stCoords[1]];
						rgb[2] += (float)stPlane->b[stCoords[0]][stCoords[1]];
						count++;
					}
				}
			}
		}
		rgb[0] /= count;
		rgb[1] /= count;
		rgb[2] /= count;
		return true;
	}
	return false;
}

void LightField::AddRayToField(Ray& ray, float* rgb) {
	
	int uvCoords[2];
	if (uvPlane->Intersect(ray, uvCoords)) {
		int stCoords[2];
		STPlane *stPlane = stPlanes[uvCoords[0] + uvPlane->xRes*uvCoords[1]];
		if (stPlane->Intersect(ray, stCoords)) {
			stPlane->r[stCoords[0]][stCoords[1]] = (short)(rgb[0] * 255);
			stPlane->g[stCoords[0]][stCoords[1]] = (short)(rgb[1] * 255);
			stPlane->b[stCoords[0]][stCoords[1]] = (short)(rgb[2] * 255);
		}
	}
}

LightField::LightField(char * fileName)
{

}

bool LightField::writeToFile()
{
    // Create lightfield file
    FILE * lightfieldBin;
	lightfieldBin = fopen("lightfield.bin", "w");
	//lightfield.open("lightfield.txt", ios::out | ios::app );
    
	//float z, width, height;
	//int xRes, yRes;

    //write UV plane properties
    fwrite((void*)(&uvPlane->z), sizeof(uvPlane->z), 1, lightfieldBin);
    fwrite((void*)(&uvPlane->width), sizeof(uvPlane->width), 1, lightfieldBin);
    fwrite((void*)(&uvPlane->height), sizeof(uvPlane->height), 1, lightfieldBin);
    fwrite((void*)(&uvPlane->xRes), sizeof(uvPlane->xRes), 1, lightfieldBin);
    fwrite((void*)(&uvPlane->yRes), sizeof(uvPlane->yRes), 1, lightfieldBin);

    //write ST plane properties
    fwrite((void*)(&stPlanes[0]->z), sizeof(stPlanes[0]->z), 1, lightfieldBin);
    fwrite((void*)(&stPlanes[0]->width), sizeof(stPlanes[0]->width), 1, lightfieldBin);
    fwrite((void*)(&stPlanes[0]->height), sizeof(stPlanes[0]->height), 1, lightfieldBin);
    fwrite((void*)(&stPlanes[0]->xRes), sizeof(stPlanes[0]->xRes), 1, lightfieldBin);
    fwrite((void*)(&stPlanes[0]->yRes), sizeof(stPlanes[0]->yRes), 1, lightfieldBin);

    //go through all st planes in raster order, then write these out to file
    for (int i = 0; i < (uvPlane->xRes)*(uvPlane->yRes); i++)
    {
        STPlane *stPlane = stPlanes[i];
        for (int ii = 0; ii < (stPlane->xRes); ii++)
        {
            for (int jj = 0; jj < (stPlane->yRes); jj++)
            {
                //short r = (short)(stPlane->r[ii][jj] * 255);
				//short g = (short)(stPlane->g[ii][jj] * 255);
				//short b = (short)(stPlane->b[ii][jj] * 255);

                short r = (short)(stPlane->r[ii][jj] );
				short g = (short)(stPlane->g[ii][jj] );
				short b = (short)(stPlane->b[ii][jj] );
                //write current stPlane rgb values to file
	            fwrite((void*)(&r), sizeof(r), 1, lightfieldBin);
				fwrite((void*)(&g), sizeof(g), 1, lightfieldBin);
				fwrite((void*)(&b), sizeof(b), 1, lightfieldBin);
            }
        }
    }
    
    fclose(lightfieldBin);
    return true;
}

bool LightField::loadFromFile()
{
 // Create lightfield file
    FILE * lightfieldBin;
	lightfieldBin = fopen("lightfield.bin", "r");
	//lightfield.open("lightfield.txt", ios::out | ios::app );
    
	//float z, width, height;
	//int xRes, yRes;

    //write UV plane properties
    fread((void*)(&uvPlane->z), sizeof(uvPlane->z), 1, lightfieldBin);
    fread((void*)(&uvPlane->width), sizeof(uvPlane->width), 1, lightfieldBin);
    fread((void*)(&uvPlane->height), sizeof(uvPlane->height), 1, lightfieldBin);
    fread((void*)(&uvPlane->xRes), sizeof(uvPlane->xRes), 1, lightfieldBin);
    fread((void*)(&uvPlane->yRes), sizeof(uvPlane->yRes), 1, lightfieldBin);

    //write ST plane properties
    fread((void*)(&stPlanes[0]->z), sizeof(stPlanes[0]->z), 1, lightfieldBin);
    fread((void*)(&stPlanes[0]->width), sizeof(stPlanes[0]->width), 1, lightfieldBin);
    fread((void*)(&stPlanes[0]->height), sizeof(stPlanes[0]->height), 1, lightfieldBin);
    fread((void*)(&stPlanes[0]->xRes), sizeof(stPlanes[0]->xRes), 1, lightfieldBin);
    fread((void*)(&stPlanes[0]->yRes), sizeof(stPlanes[0]->yRes), 1, lightfieldBin);

    //go through all st planes in raster order, then write these out to file
    for (int i = 0; i < (uvPlane->xRes)*(uvPlane->yRes); i++)
    {
        STPlane *stPlane = stPlanes[i];
        for (int ii = 0; ii < (stPlane->xRes); ii++)
        {
            for (int jj = 0; jj < (stPlane->yRes); jj++)
            {
                short r = 0;
                short g = 0;
                short b = 0;
                //write current stPlane rgb values to file
	            fread((void*)(&r), sizeof(r), 1, lightfieldBin);
				fread((void*)(&g), sizeof(g), 1, lightfieldBin);
				fread((void*)(&b), sizeof(b), 1, lightfieldBin);

                stPlane->r[ii][jj] = r;
                stPlane->g[ii][jj] = g;
                stPlane->b[ii][jj] = b;
            }
        }
    }
    
    fclose(lightfieldBin);
    return true;
}
