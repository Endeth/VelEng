#pragma once

#include "vVertices.h"
#include "vPlane.h"
#include "vLine.h"

struct CameraMatrices
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
};