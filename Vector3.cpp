#pragma once

#include "stdafx.h"
#include "Vector3.h"
#include "cPoint3D.h"


Vector3::Vector3()
{
}

Vector3::Vector3(float nx, float ny, float nz)
{
	x = nx;
	y = ny;
	z = nz;
}

Vector3::Vector3(Origin pos) {
	x = pos.x;
	y = pos.y;
	z = pos.z;
}

Vector3::Vector3(cPoint3D &point) {
	x = point.x;
	y = point.y;
	z = point.z;
}

bool Vector3::operator ==(Vector3 p3dOther)
{
	if (x != p3dOther.x) return false;
	if (y != p3dOther.y) return false;
	if (z != p3dOther.z) return false;
	return true;
}

bool Vector3::operator !=(Vector3 p3dOther)
{
	return !((x == p3dOther.x) && (y == p3dOther.y) && (z == p3dOther.z));
}

Vector3 Vector3::operator *(float fOther)
{
	return Vector3(x * fOther, y * fOther, z * fOther);
}

Vector3 Vector3::operator /(float fOther)
{
	return Vector3(x / fOther, y / fOther, z / fOther);
}

Vector3 Vector3::operator +(Vector3 p3dOther)
{
	return Vector3(x + p3dOther.x, y + p3dOther.y, z + p3dOther.z);
}

Vector3 Vector3::operator -(Vector3 p3dOther)
{
	return Vector3(x - p3dOther.x, y - p3dOther.y, z - p3dOther.z);
}

void Vector3::operator *=(float fOther)
{
	x *= fOther;
	y *= fOther;
	z *= fOther;

	return; //true;
}

void Vector3::operator /=(float fOther)
{
	x /= fOther;
	y /= fOther;
	z /= fOther;

	return; //true;
}

void Vector3::operator +=(Vector3 p3dOther)
{
	x += p3dOther.x;
	y += p3dOther.y;
	z += p3dOther.z;

	return; //true;
}

void Vector3::operator -=(Vector3 p3dOther)
{
	x -= p3dOther.x;
	y -= p3dOther.y;
	z -= p3dOther.z;

	return; //true;
}

void Vector3::RotateAround(Vector3 Center, Vector3 Rotation)
{
	float tpx, tpy, tpz;

	x -= Center.x;
	y -= Center.y;
	z -= Center.z;

	tpy = y * (float)cos(Rotation.x) - z * (float)sin(Rotation.x);
	tpz = z * (float)cos(Rotation.x) + y * (float)sin(Rotation.x);

	z = tpz * (float)cos(Rotation.y) - x * (float)sin(Rotation.y) + Center.z;
	tpx = x * (float)cos(Rotation.y) + tpz * (float)sin(Rotation.y);

	x = tpx * (float)cos(Rotation.z) - tpy * (float)sin(Rotation.z) + Center.x;
	y = tpy * (float)cos(Rotation.z) + tpx * (float)sin(Rotation.z) + Center.y;
}

Vector3 Vector3::Cross(Vector3 p3dOther)
{
	return Vector3::Vector3(y*p3dOther.z - p3dOther.y*z, z*p3dOther.x - p3dOther.z*x, x*p3dOther.y - p3dOther.x*y);
}

float Vector3::DotProduct(Vector3 p3dOther)
{
	return x * p3dOther.x + y * p3dOther.y + z * p3dOther.z;
}

float Vector3::Dot2D(Vector3 p3dOther) {
	return x * p3dOther.x + y * p3dOther.y;
}

void Vector3::Normalize()
{
	float fTemp = (float)Abs();
	x /= fTemp;
	y /= fTemp;
	z /= fTemp;
}

void Vector3::CalcFromLocation(stLocation *Loc)
{
	z = Loc->Origin.z * MODEL_SCALE_FACTOR;

	DWORD dwBlockX = (Loc->cell_id & 0xff000000) >> 24;
	DWORD dwBlockY = (Loc->cell_id & 0x00ff0000) >> 16;

	if (dwBlockX < 3)
	{
		// dungeon, dwBlockX and dwBlockY remain constant - just use x and y
		x = Loc->Origin.x * MODEL_SCALE_FACTOR;
		y = Loc->Origin.y * MODEL_SCALE_FACTOR;
	}
	else
	{
		// outdoors
		x = (float)((((float(dwBlockX) + 1.0f) * 8.0f + (Loc->Origin.x / 24.0)) - 1027.5) / 10.0f);
		y = (float)((((float(dwBlockY) + 1.0f) * 8.0f + (Loc->Origin.y / 24.0)) - 1027.5) / 10.0f);
	}
}

float Vector3::Abs()
{
	return (float)sqrt(x*x + y * y + z * z);
}

