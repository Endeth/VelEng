#pragma once
#include <vector>
#include <memory>

#include "glew.h"
#include "glm/glm.hpp"
#include "../../VShaders/VGLSLShader.h"
#include "../../VGeo/VGeo.h"
#include "../../VBuffers/VBuffers.h"
#include "../../VDrawable/VDrawableObject.h"
#include "../../VTextures/VTexture.h"
#include "../../VMaterials/VMaterial.h"

namespace Vel
{

	class VMesh : public VBasicDrawableObject
	{
	public:
		VMesh();
		virtual ~VMesh();
		
		void LoadMesh(const char* filename = " ");
		void SetMaterial(const std::shared_ptr<VMaterial>& mat);
		void DeleteVertices();

		virtual void LoadIntoGPU();

		const GLsizei GetIndicesCount() const { return _indices.size(); }
		

	protected:
		void UpdateVerticesInGPU();

		virtual void SetVAO();

		virtual void BindAdditionalDrawingOptions();
		virtual void UnbindAdditionalDrawingOptions();

		VArrayBuffer _vboVertices;
		VElementArrayBuffer _vboIndices;
		std::shared_ptr<VMaterial> _material;
		bool _isLoaded{ false };
	};

	class VPlaneMesh : public VMesh
	{
	public:
		VPlaneMesh();
		void LoadMesh(const char* filename = " ");
	};



}