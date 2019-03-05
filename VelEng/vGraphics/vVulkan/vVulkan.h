#pragma once

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vVulkanBuffer.h"
#include "vVulkanCommands.h"
#include "vVulkanDevice.h"
#include "vVulkanSwapChain.h"
#include "vVulkanRenderpass.h"
#include "vVulkanImage.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
    class Vulkan
    {
    public:
        void Init( GLFWwindow *window );
        void Destroy();
		void CreateSurface( GLFWwindow *window );
		void CreateCommandBuffers();
		void DestroyCommandBuffers();
		void RecordCommandBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateStagingBuffer();
		void CreateBuffer();
		void CreateImage();
		void CreateDepthImage();
		void CreateUniformBuffers();
		void UpdateUniformBuffers( uint32_t imageIndex );
		void UpdateCamera( glm::mat4 &view, glm::mat4 &proj );
		void Draw();

        VulkanDeviceManager _deviceManager;
        VulkanSwapchain _swapchain;
		VulkanRenderPass _renderPass;

		VkCommandPool _commandPoolGraphics = VK_NULL_HANDLE; //TODO get rid of this test cmdpool
		VkCommandPool _commandPoolTransfer = VK_NULL_HANDLE; //TODO get rid of this test cmdpool
		VkDescriptorSetLayout _descriptorSetLayout; //TODO move descriptors to a single class ( shader -> attributes + descriptors )
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE; // test dsptrPool
		std::vector<VkDescriptorSet> _descriptorSets;
		std::vector<VkCommandBuffer> _commandBuffers;
		VkCommandBuffer _transferCommand;
		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;
		VulkanBuffer _stagingBuffer;
		VulkanImage _sampledImage;
		VulkanImage _depthImage;
		std::vector<VulkanBuffer> _uniformBuffers;
		VkPipelineLayout _pipelineLayout; //TODO nasty hack to bind descriptor sets

        bool _enableValidationLayers = false;
		CameraMatrices _internalCamera;
    };
}