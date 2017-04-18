#pragma once
#include <vector>
#include <memory>

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include "../VShaders/VGLSLShader.h"
#include "../VGeo/VGeo.h"
#include "../VBuffers/VBuffers.h"
#include "../VDrawable/VDrawableObject.h"
#include "../VTextures/VTexture.h"
#include "../VMaterials/VMaterial.h"

namespace Vel
{

	class Mesh : public BasicDrawableObject
	{
	public:
		Mesh();
		virtual ~Mesh();
		
		void LoadMesh(const char* filename = " ");
		void LoadVerticesOnly();
		void SetMaterial(const std::shared_ptr<Material>& mat);
		void DeleteVertices();

		virtual void LoadIntoGPU();

		const GLsizei GetIndicesCount() const { return _indices.size(); }
		

	protected:
		void UpdateVerticesInGPU();

		virtual void SetVAO() override;
		void SetVerticesVAO(); //DEBUG (not even in base class) though might be useful
		virtual void BindAdditionalDrawingOptions() override;
		virtual void UnbindAdditionalDrawingOptions() override;

		ArrayBuffer _vboVertices;
		ElementArrayBuffer _vboIndices;
		std::shared_ptr<Material> _material;
		bool _isLoaded{ false };
	};

	class PlaneMesh : public Mesh
	{
	public:
		PlaneMesh();
		void LoadMesh(const char* filename = " ");
		void LoadVerticesOnly();
	};



}