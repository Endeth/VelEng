#pragma once
#include <memory>

#include "VMesh.h"


namespace Vel
{
	
	class VSkybox : public VMesh
	{
	public:
		VSkybox(std::shared_ptr<VSkyboxTexture> texture);
		void SetVAO();
	private:
		void BindAdditionalDrawingOptions() final;
		void UnbindAdditionalDrawingOptions() final;
		std::shared_ptr<VTexture> _skyTex;
	};
}