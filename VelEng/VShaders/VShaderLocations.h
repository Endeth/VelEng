#pragma once

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include "../VOpenGL/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace Vel
{
	namespace AttributeLocation
	{
		enum GPass
		{
			vVertex = 0,
			vNormal = 0,
			vUV = 0
		};
		enum LPass
		{
			vVertex = 0,
			vUV = 1
		};
		enum ShadowMapping
		{
			vVertex = 0,
			vNormal = 0,
			vUV = 0
		};
		enum ShadowsCube
		{
			vVertex = 0,
			vNormal = 0,
			vUV = 0
		};
		enum Skybox
		{
			vVertex = 0
		};
	}
	namespace UniformLocation
	{
		enum GPass
		{
			M = 0,
			V = 1,
			P = 2
		};
		enum LPass
		{
			vVertex = 0,
			vUV = 1
		};
		enum ShadowMapping
		{
			M = 0,
			lightSpaceMatrix = 1
		};
		enum ShadowsCube
		{
			M = 0
		};
		enum Skybox
		{
			M = 0,
			V = 1,
			P = 2
		};
	}
}