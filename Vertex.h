#pragma once

//** Ported from ACE: https://github.com/ACEmulator/ACE

#include "Vector3.h"

namespace Physics
{
	namespace Entity {
		enum class VertexType
		{
			Unknown = 0x0,
			CSWVertexType = 0x1
		};


		class Vertex
		{
		public:
			unsigned short Index;
			Vector3 Origin;
			Vector3 Normal;

			Vertex() { }

			Vertex(unsigned short index)
			{
				Index = index;
			}

			// TODO: implement me
			//Vertex(DatLoader.Entity.SWVertex v)
			//{
			//	Origin = v.Origin;
			//	Normal = v.Normal;
			//
			//	// omitted UV texture coordinates
			//}

			Vertex(Vector3 origin)
			{
				Origin = origin;
			}

			Vector3 operator+ (Vertex b)
			{
				return Origin + b.Origin;
			}

			Vector3 operator- (Vertex b)
			{
				return Origin - b.Origin;
			}

			// TODO: implement me (needs Vector3 operators)
			//Vector3 operator* (Vertex b)
			//{
			//	return Origin * b.Origin;
			//}
			//
			//Vector3 operator/ (Vertex b)
			//{
			//	return Origin / b.Origin;
			//}

			bool Equals(Vertex v)
			{
				return Index == v.Index && Origin.x == v.Origin.x && Origin.y == v.Origin.y && Origin.z == v.Origin.z &&
					Normal.x == v.Normal.x && Normal.y == v.Normal.y && Normal.z == v.Normal.z;
			}

			// TODO: implement me
			//int GetHashCode()
				//{
				//	int hash = 0;
				//
				//	hash = (hash * 397) ^ Index.GetHashCode();
				//	hash = (hash * 397) ^ Origin.GetHashCode();
				//	hash = (hash * 397) ^ Normal.GetHashCode();
				//
				//	return hash;
				//}
		};
	}
}
