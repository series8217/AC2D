#pragma once

#include "stdafx.h"

class Vector3;

class cPoint3D {
public:
	cPoint3D();
	cPoint3D(float nx, float ny, float nz);
	cPoint3D(Origin pos);
	cPoint3D(Vector3 &vec);

	bool operator ==(cPoint3D p3dOther);
	bool operator !=(cPoint3D p3dOther);
	cPoint3D operator *(float fOther);
	cPoint3D operator /(float fOther);
	cPoint3D operator +(cPoint3D p3dOther);
	cPoint3D operator -(cPoint3D p3dOther);
	void operator *=(float fOther);
	void operator /=(float fOther);
	void operator +=(cPoint3D p3dOther);
	void operator -=(cPoint3D p3dOther);

	void RotateAround(cPoint3D Center, cPoint3D Rotation);
	cPoint3D Cross(cPoint3D p3dOther);
	float DotProduct(cPoint3D p3dOther);
	float Dot2D(cPoint3D p3dOther);
	void Normalize();

	void CalcFromLocation(stLocation *Loc);
	float Abs();
	float	x, y, z;
};

