#pragma once
#include <memory>

#include "vVulkanCommon.h"
#include "vVulkanBuffer.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
	class TexelData
	{
	public:
		TexelData( const char *path );
		~TexelData();
		size_t GetSize();

		void *_data = nullptr;
		glm::ivec2 _imageSize;
		int _imageChannels;
	};

	class VulkanSamplers
	{
	public:
		enum Type
		{
			BasicSampler = 0,
			TYPES = 1
		};

		void CreateSamplers();
		void DestroySamplers();
		VkSampler GetSampler( Type samplerType );

		std::vector<VkSampler> _samplers;
	};
	static VulkanSamplers Samplers; //TODO

	class VulkanImage
	{
	public:
		void Create( TexelData &data, VkImageCreateFlags flags, VkImageUsageFlags usage, VkSharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices );
		void AllocateMemory( uint32_t memoryTypeIndex );
		void Destroy();

		//This sucks. Maybe add respective VkMemoryBarriers to arrays which we use in a single vkCmdPipelineBarrier each frame and get rid of cmdPool and queue
		void AdjustMemoryBarrier( VkImageLayout newLayout, VkCommandPool cmdPool, VkQueue queue );

		glm::ivec2 _imageSize;

		VkImage _image = VK_NULL_HANDLE;
		VkImageView _imageView = VK_NULL_HANDLE;
		VkSampler _sampler = VK_NULL_HANDLE;
		VkDeviceMemory _imageMemory = VK_NULL_HANDLE;
		VkMemoryRequirements _memoryRequirements;

		VkAccessFlags _accessMask = 0;
		VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	void CopyImage( VulkanImage &srcImage, VulkanImage &dstImage, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue );
	void CopyImage( VulkanBuffer &srcBuffer, VulkanImage &dstImage, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue );
}