#pragma once

#include "stdafx.h"
#include "Vector2.h"


Vector2::Vector2()
{
}

Vector2::Vector2(float nx, float ny)
{
	x = nx;
	y = ny;
}



bool Vector2::operator ==(Vector2 p2dOther)
{
	if (x != p2dOther.x) return false;
	if (y != p2dOther.y) return false;
	return true;
}

bool Vector2::operator !=(Vector2 p2dOther)
{
	return !((x == p2dOther.x) && (y == p2dOther.y));
}

Vector2 Vector2::operator *(float fOther)
{
	return Vector2(x * fOther, y * fOther);
}

Vector2 Vector2::operator /(float fOther)
{
	return Vector2(x / fOther, y / fOther);
}

Vector2 Vector2::operator +(Vector2 p2dOther)
{
	return Vector2(x + p2dOther.x, y + p2dOther.y);
}

Vector2 Vector2::operator -(Vector2 p2dOther)
{
	return Vector2(x - p2dOther.x, y - p2dOther.y);
}

void Vector2::operator *=(float fOther)
{
	x *= fOther;
	y *= fOther;

	return; //true;
}

void Vector2::operator /=(float fOther)
{
	x /= fOther;
	y /= fOther;

	return; //true;
}

void Vector2::operator +=(Vector2 p2dOther)
{
	x += p2dOther.x;
	y += p2dOther.y;

	return; //true;
}

void Vector2::operator -=(Vector2 p2dOther)
{
	x -= p2dOther.x;
	y -= p2dOther.y;

	return; //true;
}

float Vector2::Dot(Vector2 p2dOther) {
	return x * p2dOther.x + y * p2dOther.y;
}

void Vector2::Normalize()
{
	float fTemp = (float)Abs();
	x /= fTemp;
	y /= fTemp;
}

float Vector2::Abs()
{
	return (float)sqrt(x*x + y * y);
}

