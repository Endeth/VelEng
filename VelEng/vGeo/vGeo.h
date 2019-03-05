#pragma once

#include "vVertices.h"
#include "vPlane.h"
#include "vLine.h"

struct CameraMatrices
{
	alignas( 16 ) glm::mat4 Model;
	alignas( 16 ) glm::mat4 View;
	alignas( 16 ) glm::mat4 Projection;
};