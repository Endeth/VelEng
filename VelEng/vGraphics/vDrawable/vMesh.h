#pragma once

#include <vector>
#include <memory>

#include "external/glm/glm.hpp"

#include "vGeo/vGeo.h"
#include "vGraphics/vDrawable/vDrawableObject.h"
#include "vGraphics/vMaterials/vMaterial.h"

namespace Vel
{

	class Mesh : public BasicDrawableObject
	{
	public:
		Mesh() {};
		Mesh( const tinyobj::shape_t &shape, const tinyobj::attrib_t &attrib );
		virtual ~Mesh();

		void SetMaterial(const std::shared_ptr<Material>& mat);

		virtual void LoadIntoGPU();

		const size_t GetIndicesCount() const { return _indices.size(); }
		

	//protected:
		void UpdateVerticesInGPU();

		virtual void SetVAO() override;
		void SetVerticesVAO(); //DEBUG (not even in base class) though might be useful
		virtual void BindAdditionalDrawingOptions() override;
		virtual void UnbindAdditionalDrawingOptions() override;

		//ArrayBuffer _vboVertices;
		//ElementArrayBuffer _vboIndices;
		std::shared_ptr<Material> _material;
		bool _isLoaded{ false };
	};
}