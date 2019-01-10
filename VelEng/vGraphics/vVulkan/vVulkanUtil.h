#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include <vulkan/vulkan.hpp>

#include "VelEngConfig.h"

namespace Vel
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct Semaphores
	{
		void CreateSemaphores( VkDevice device );

		VkSemaphore presentComplete = 0;
		VkSemaphore renderComplete = 0;
		VkSemaphore acquireComplete = 0;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphics = 0;
		uint32_t present = 0;
		uint32_t compute = 0;
		uint32_t transfer = 0;
	};

	void CheckResult( VkResult result, const char *failMsg );

	template< typename FirstObjectToQuery, typename SecondObjectToQuery, typename DataType, typename Result >
	Result VulkanQuery( FirstObjectToQuery fObj, SecondObjectToQuery sObj, std::function<Result( FirstObjectToQuery, SecondObjectToQuery, uint32_t*, DataType* )> VkQueryFunc, std::vector<DataType> &dataHolder )
	{
		uint32_t dataCount;

		VkQueryFunc( fObj, sObj, &dataCount, nullptr );
		if( dataCount == 0 )
			throw std::runtime_error( "failed query" );

		dataHolder.resize( dataCount );
		return VkQueryFunc( fObj, sObj, &dataCount, dataHolder.data() );
	}

    template< typename ObjectToQuery, typename DataType, typename Result >
    Result VulkanQuery( ObjectToQuery obj, std::function<Result( ObjectToQuery, uint32_t*, DataType*)> VkQueryFunc, std::vector<DataType> &dataHolder )
    {
        uint32_t dataCount;

        VkQueryFunc( obj, &dataCount, nullptr );
        if ( dataCount == 0 )
            throw std::runtime_error( "failed query" );

        dataHolder.resize( dataCount );
        return VkQueryFunc( obj, &dataCount, dataHolder.data() );
    }

    template< typename ObjectToQuery, typename DataType >
    void VulkanQuery( ObjectToQuery obj, std::function<void( ObjectToQuery, uint32_t*, DataType* )> VkQueryFunc, std::vector<DataType> &dataHolder )
    {
        uint32_t dataCount;

        VkQueryFunc( obj, &dataCount, nullptr );
        if ( dataCount == 0 )
            throw std::runtime_error( "failed query" );

        dataHolder.resize( dataCount );
        VkQueryFunc( obj, &dataCount, dataHolder.data() );
    }

    template< typename DataType >
    VkResult VulkanQuery( std::function<VkResult( uint32_t*, DataType* )> VkQueryFunc, std::vector<DataType> &dataHolder )
    {
        uint32_t dataCount;

        VkQueryFunc( &dataCount, nullptr );
        if ( dataCount == 0 )
            throw std::runtime_error( "failed query" );

        dataHolder.resize( dataCount );
        return VkQueryFunc( &dataCount, dataHolder.data() );
    }
}