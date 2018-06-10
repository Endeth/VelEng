#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include "external/vulkan/vulkan.h"

#include "VelEngConfig.h"

namespace Vel
{
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