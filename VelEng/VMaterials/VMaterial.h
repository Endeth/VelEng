#pragma once

#include <memory>

#include "../VOpenGL/glm/glm.hpp"
#include "../VTextures/VTexture.h"

namespace Vel
{
	class Material 
	{
	protected:
		using VTexturePtr = std::shared_ptr<Vel::Texture>;
	public:
		Material(const VTexturePtr& diffuse, const VTexturePtr& specular, GLfloat& shininess);
		void SetTexturesUnits();
		virtual void BindMaterial();
		virtual void UnbindMaterial();
	private:
		std::shared_ptr<Texture> _diffuse;
		std::shared_ptr<Texture> _specular;
		GLfloat _shininess;
	};

	class EmissiveMaterial : public Material
	{
	public:
		EmissiveMaterial(const VTexturePtr& diffuse, const VTexturePtr& specular, const VTexturePtr& emission, GLfloat& shininess);
		void BindMaterial() final;
		void UnbindMaterial() final;
	protected:
		std::shared_ptr<Texture> _emission;
	};
}

