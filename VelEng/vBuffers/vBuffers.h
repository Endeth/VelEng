#pragma once

#include <memory>
#include <vector>

#include "vGeo/vVertices.h"

namespace Vel
{
	template <typename BufferDataPtr>
	class Buffer
	{
	public:
		virtual ~Buffer()
        {	
		}
		void BindBuffer(uint16_t BufferNumber = 0)
		{
		}
		void UnbindBuffer()
		{
		}
		void FillBuffer(BufferDataPtr Data, uint16_t BufferNumber = 0)
		{
		}
		void FillBuffer(int32_t NumberOfElements, BufferDataPtr Data, uint16_t BufferNumber = 0)
		{
		}

	protected:
		Buffer(uint16_t BuffersCount = 1)
		{
		}
		Buffer(int32_t NumberOfElements, uint16_t BuffersCount = 1)
		{
		}

		uint32_t *_buffersID;
		uint16_t _buffersCount;
		uint32_t _bufferType;
		uint32_t _bufferUsage;
		int32_t _buffersSize;

	};

	class ArrayBuffer : public Buffer<Vertex*>
	{
	public:
		ArrayBuffer(uint16_t BuffersCount = 1, uint32_t BufferUsage = 0) : Buffer(BuffersCount)
		{
		}
		ArrayBuffer(int32_t NumberOfElements, uint16_t BuffersCount = 1, uint32_t BufferUsage = 0) : Buffer(NumberOfElements, BuffersCount)
		{
		}
	};

	class ElementArrayBuffer : public Buffer<uint32_t*>
	{
	public:
		ElementArrayBuffer(uint16_t BuffersCount = 1, uint32_t BufferUsage = 0) : Buffer(BuffersCount)
		{
		}
		ElementArrayBuffer(int32_t NumberOfElements, uint16_t BuffersCount = 1, uint32_t BufferUsage = 0) : Buffer(NumberOfElements, BuffersCount)
		{
		}
	};
}