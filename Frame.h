#pragma once


#include "quaternion.h"
#include "Vector3.h"

//** Ported from ACE: https://github.com/ACEmulator/ACE
namespace Entity
{
    class Frame
    {
    public:
        Vector3 Origin;
        Quaternion Orientation;

        Frame()
        {
            Origin = Vector3();
            Orientation = Quaternion();
        }
	};
}
