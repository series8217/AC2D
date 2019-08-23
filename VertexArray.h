#pragma once

//** Ported from ACE: https://github.com/ACEmulator/ACE

#include "Vertex.h"

using namespace Physics::Entity;

namespace Physics {
	namespace Common {
		class VertexArray
		{
		public:
			VertexType Type;
			std::vector<Vertex> Vertices;

			VertexArray() { Type = VertexType::Unknown; }

			VertexArray(VertexType type, int numVerts)
			{
				for (int i = 0; i < numVerts; i++) {
					Vertices.push_back(Vertex());
				}
				Type = type;
			}
		};
	}
}