//** Ported from ACE: https://github.com/ACEmulator/ACE

#pragma once

#include "Vector3.h"

namespace Physics {
    class Plane
    {
    public:
		// Distance of the plane along its normal from the origin
        float D;
		
		Vector3 Normal;

		Plane(Vector3 normal, float d) {
			Normal = normal;
			Normal.Normalize();
			D = d;
		}
    };
}