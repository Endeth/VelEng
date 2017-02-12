#pragma once
#include <memory>

#include "VMesh.h"


namespace Vel
{
	
	class VSkybox : public VMesh
	{
	public:
		VSkybox(const  std::shared_ptr<VTextureCube> &texture);
		void SetVAO();
	private:
		void BindAdditionalDrawingOptions() final;
		void UnbindAdditionalDrawingOptions() final;
		std::shared_ptr<VTexture> _skyTex;
	};
}