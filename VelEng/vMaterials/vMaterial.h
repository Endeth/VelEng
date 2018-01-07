#pragma once

#include <memory>

#include "external/glm/glm.hpp"
#include "vTextures/vTexture.h"

namespace Vel
{
	class Material 
	{
	protected:
		using VTexturePtr = std::shared_ptr<Vel::Texture>;
	public:
		Material(const VTexturePtr& diffuse, const VTexturePtr& normalMap, const VTexturePtr& specular, float& shininess);
		void SetTexturesUnits();
		virtual void BindMaterial();
		virtual void UnbindMaterial();
	private:
		VTexturePtr _diffuse;
		VTexturePtr _normalMap;
		VTexturePtr _specular;
		float _shininess;
	};

	class EmissiveMaterial : public Material
	{
	public:
		EmissiveMaterial(const VTexturePtr& diffuse, const VTexturePtr& normalMap, const VTexturePtr& specular, const VTexturePtr& emission, float& shininess);
		void BindMaterial() final;
		void UnbindMaterial() final;
	protected:
		VTexturePtr _emission;
	};
}

