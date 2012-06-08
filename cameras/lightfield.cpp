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
	
	r = new float*[xRes];
	g = new float*[xRes];
	b = new float*[xRes];
	for (int i = 0; i < xRes; i++) {
		r[i] = new float[yRes];
		g[i] = new float[yRes];
		b[i] = new float[yRes];
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
		int stCoords[2];
		STPlane *stPlane = stPlanes[uvCoords[0] + uvPlane->xRes*uvCoords[1]];
		if (stPlane->Intersect(ray, stCoords)) {
			rgb[0] = stPlane->r[stCoords[0]][stCoords[1]] * 5;
			rgb[1] = stPlane->g[stCoords[0]][stCoords[1]] * 5;
			rgb[2] = stPlane->b[stCoords[0]][stCoords[1]] * 5;
			return true;
		}
	}
	return false;
}

void LightField::AddRayToField(Ray& ray, float* rgb) {
	
	int uvCoords[2];
	if (uvPlane->Intersect(ray, uvCoords)) {
		int stCoords[2];
		STPlane *stPlane = stPlanes[uvCoords[0] + uvPlane->xRes*uvCoords[1]];
		if (stPlane->Intersect(ray, stCoords)) {
			stPlane->r[stCoords[0]][stCoords[1]] = rgb[0];
			stPlane->g[stCoords[0]][stCoords[1]] = rgb[1];
			stPlane->b[stCoords[0]][stCoords[1]] = rgb[2];
		}
	}
}