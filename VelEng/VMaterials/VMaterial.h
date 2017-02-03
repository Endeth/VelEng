#pragma once

#include <memory>

#include "../VOpenGL/glm/glm.hpp"
#include "../VTextures/VTexture.h"

namespace Vel
{
	class VMaterial 
	{
	protected:
		using VTexturePtr = std::shared_ptr<Vel::VTexture>;
	public:
		VMaterial(const VTexturePtr& diffuse, const VTexturePtr& specular, GLfloat& shininess);
		void SetTexturesUnits();
		virtual void BindMaterial();
		virtual void UnbindMaterial();
	private:
		std::shared_ptr<VTexture> _diffuse;
		std::shared_ptr<VTexture> _specular;
		GLfloat _shininess;
	};

	class VEmissiveMaterial : public VMaterial
	{
	public:
		VEmissiveMaterial(const VTexturePtr& diffuse, const VTexturePtr& specular, const VTexturePtr& emission, GLfloat& shininess);
		void BindMaterial() final;
		void UnbindMaterial() final;
	protected:
		std::shared_ptr<VTexture> _emission;
	};
}

