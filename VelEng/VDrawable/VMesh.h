#pragma once
#include <vector>
#include <memory>

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "VGLSLShader.h"
#include "VGeo.h"
#include "VBuffers.h"
#include "VDrawableObject.h"
#include "VTexture.h"
#include "VMaterial.h"

namespace Vel
{

	class VMesh : public VBasicDrawableObject
	{
	public:
		VMesh();
		virtual ~VMesh();
		
		void LoadMesh(const char* filename = " ");
		void AddMaterial(const std::shared_ptr<VMaterial>& mat);
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