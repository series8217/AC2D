#pragma once

#include "stdafx.h"

class Vector2 {
public:
	Vector2();
	Vector2(float nx, float ny);

	bool operator ==(Vector2 p2dOther);
	bool operator !=(Vector2 p2dOther);
	Vector2 operator *(float fOther);
	Vector2 operator /(float fOther);
	Vector2 operator +(Vector2 p2dOther);
	Vector2 operator -(Vector2 p2dOther);
	void operator *=(float fOther);
	void operator /=(float fOther);
	void operator +=(Vector2 p2dOther);
	void operator -=(Vector2 p2dOther);

	float Dot(Vector2 p2dOther);
	void Normalize();

	float Abs();
	float	x, y;
};
