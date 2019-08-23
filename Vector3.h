#pragma once

#include "stdafx.h"

class cPoint3D;

class Vector3 {
public:
	Vector3();
	Vector3(float nx, float ny, float nz);
	Vector3(Origin pos);
	Vector3(cPoint3D &point);

	bool operator ==(Vector3 p3dOther);
	bool operator !=(Vector3 p3dOther);
	Vector3 operator *(float fOther);
	Vector3 operator /(float fOther);
	Vector3 operator +(Vector3 p3dOther);
	Vector3 operator -(Vector3 p3dOther);
	void operator *=(float fOther);
	void operator /=(float fOther);
	void operator +=(Vector3 p3dOther);
	void operator -=(Vector3 p3dOther);

	void RotateAround(Vector3 Center, Vector3 Rotation);
	Vector3 Cross(Vector3 p3dOther);
	float DotProduct(Vector3 p3dOther);
	float Dot2D(Vector3 p3dOther);
	void Normalize();

	void CalcFromLocation(stLocation *Loc);
	float Abs();
	float	x, y, z;
};
