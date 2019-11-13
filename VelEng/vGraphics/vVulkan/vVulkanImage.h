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

		void *data = nullptr;
		glm::ivec2 imageSize;
		int imageChannels;
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

		std::vector<VkSampler> samplers;
	};
	static VulkanSamplers Samplers; //TODO

	class VulkanImage //TODO create children for sampled texture, depthbuffer etc
	{
	public:
		void Create( glm::ivec2 size, VkDeviceSize deviceSize, VkFormat imageFormat, VkImageCreateFlags flags, VkImageUsageFlags usage, VkSharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices );
		void AllocateMemory( uint32_t memoryTypeIndex );
		void Destroy();

		//TODO This sucks. Maybe add respective VkMemoryBarriers to arrays which we use in a single vkCmdPipelineBarrier each frame and get rid of cmdPool and queue
		void AdjustMemoryBarrier( VkImageLayout newLayout, VkCommandPool cmdPool, VkQueue queue );

		glm::ivec2 imageSize = glm::ivec2( 0 );

		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkMemoryRequirements memoryRequirements;

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkAccessFlags accessMask = 0;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	void CopyImage( VulkanImage &srcImage, VulkanImage &dstImage, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue );
	void CopyImage( VulkanBuffer &srcBuffer, VulkanImage &dstImage, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue );
}