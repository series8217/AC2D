//** Ported from ACE: https://github.com/ACEmulator/ACE

#pragma once

#include "stdafx.h"
#include "Vector3.h"
#include "Vertex.h"
#include "Plane.h"

using namespace Physics::Entity;

namespace Physics {

    /// <summary>
/// Polygon culling mode
/// </summary>
    enum class CullMode
    {
        Landblock = 0x0,     // not in dat, but it seems to be used for this?
        None = 0x1,
        Clockwise = 0x2,
        CounterClockwise = 0x3
    };

    // TODO: put this somewhere else?
    enum class Sidedness
    {
        Positive = 0x0,
        Negative = 0x1,
        InPlane = 0x2,
        Crossing = 0x3
    };

    class Polygon {
    public:
        std::vector<Vertex> Vertices;
        std::vector<short> VertexIDs;
        int PolyID;
        int NumPoints;
        CullMode SidesType;
        short PosSurface;
        short NegSurface;
        Plane* Plane;

        Polygon(int idx, int numPoints, CullMode cullMode) {
            PolyID = idx;
            NumPoints = numPoints;
            SidesType = cullMode;
			Plane = NULL;

            for (int i = 0; i < numPoints; i++)
            {
                Vertices.push_back(Vertex());
                VertexIDs.push_back(-1);
            }
        }

		~Polygon() {
			if (Plane) {
				delete Plane;
			}
			Vertices.clear();
			VertexIDs.clear();
		}

        bool point_in_poly2D(Vector3 point, Sidedness side)
        {
            int prevIdx = 0;
            for (int i = NumPoints - 1; i >= 0; i--)
            {
				Vertex* prevVertex = &Vertices[prevIdx];
				Vertex* vertex = &Vertices[i];

                Vector3 diff = *vertex - *prevVertex;

                // 2d cross product difference?
                float diffCross = (diff.y * vertex->Origin.x) - (diff.x * vertex->Origin.y) + \
														(diff.x * point.y) - (diff.y * point.x);

                if (side != Sidedness::Positive)
                {
                    if (diffCross < 0.0f)
                        return false;
                }
                else
                {
                    if (diffCross > 0.0f)
                        return false;
                }
                prevIdx = i;
            }
            return true;
        }

		void make_plane()
		{
			Vector3 normal = Vector3(0.0, 0.0, 0.0);

			// calculate plane normal
			for (int i = NumPoints - 2, spreadIdx = 1; i > 0; i--)
			{
				Vector3 v1 = Vertices[spreadIdx++] - Vertices[0];
				Vector3 v2 = Vertices[spreadIdx] - Vertices[0];

				normal += v1.Cross(v2);
			}
			normal.Normalize();

			// calculate distance
			float distSum = 0.0f;
			for (int i = NumPoints, spread = 0; i > 0; i--, spread++) {
				distSum += normal.DotProduct(Vertices[spread].Origin);
			}

			float dist = -(distSum / NumPoints);

			Plane = new Physics::Plane(normal, dist);
		}
    };
}
