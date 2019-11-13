#pragma once

#include <memory>

#include "external/glm/glm.hpp"

#include "vGraphics/vTextures/vTexture.h"
#include "vMaterialPipeline.h"

namespace Vel
{
	class Material
	{
	protected:
		using TexturePtr = std::shared_ptr<Vel::Texture>;
	public:
		Material();

		virtual void BindMaterial();
		virtual void UnbindMaterial();
		MaterialPipeline pipeline;
	};

	class PhongMaterial : public Material
	{
	public:
		PhongMaterial( const TexturePtr& diffuse, const TexturePtr& normalMap, const TexturePtr& specular, float& shininess );
	private:
		TexturePtr _diffuse;
		TexturePtr _normalMap;
		TexturePtr _specular;
		float _shininess;
	};

	class EmissiveMaterial : public PhongMaterial
	{
	public:
		EmissiveMaterial(const TexturePtr& diffuse, const TexturePtr& normalMap, const TexturePtr& specular, const TexturePtr& emission, float& shininess);
		void BindMaterial() final;
		void UnbindMaterial() final;
	protected:
		TexturePtr _emission;
	};
}

