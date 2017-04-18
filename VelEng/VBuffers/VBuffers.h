#pragma once

#include "../VOpenGL/glew.h"
#include "../VGeo/VVertices.h"
#include <memory>
#include <vector>

namespace Vel
{
	template <typename BufferDataPtr>
	class Buffer
	{
	public:
		virtual ~Buffer()
	{	
			glDeleteBuffers(_buffersCount, _buffersID);
			delete[] _buffersID;
		}
		void BindBuffer(GLushort BufferNumber = 0)
		{
			glBindBuffer(_bufferType, _buffersID[BufferNumber]);
		}
		void UnbindBuffer()
		{
			glBindBuffer(_bufferType, 0);
		}
		virtual void FillBuffer(BufferDataPtr Data, GLushort BufferNumber = 0)
		{
			glBindBuffer(_bufferType, _buffersID[BufferNumber]);
			glBufferData(_bufferType, _buffersSize, Data, _bufferUsage);
		}
		virtual void FillBuffer(GLsizei NumberOfElements, BufferDataPtr Data, GLushort BufferNumber = 0)
		{
			_buffersSize = NumberOfElements * sizeof(Vertex);
			glBindBuffer(_bufferType, _buffersID[BufferNumber]);
			glBufferData(_bufferType, _buffersSize, Data, _bufferUsage);
		}

	protected:
		Buffer(GLushort BuffersCount = 1)
		{
			_buffersCount = BuffersCount;
			_buffersID = new GLuint[_buffersCount];
			glGenBuffers(_buffersCount, _buffersID);
		}
		Buffer(GLsizei NumberOfElements, GLushort BuffersCount = 1)
		{
			_buffersCount = BuffersCount;
			_buffersSize = NumberOfElements;
			_buffersID = new GLuint[_buffersCount];
			glGenBuffers(_buffersCount, _buffersID);
		}

		GLuint *_buffersID;
		GLushort _buffersCount;
		GLenum _bufferType;
		GLenum _bufferUsage;
		GLsizei _buffersSize;

	};

	class ArrayBuffer : public Buffer<Vertex*>
	{
	public:
		ArrayBuffer(GLushort BuffersCount = 1, GLenum BufferUsage = GL_STATIC_DRAW) : Buffer(BuffersCount)
		{
			_bufferType = GL_ARRAY_BUFFER;
			_bufferUsage = BufferUsage;
			_buffersSize = sizeof(Vertex);
		}
		ArrayBuffer(GLsizei NumberOfElements, GLushort BuffersCount = 1, GLenum BufferUsage = GL_STATIC_DRAW) : Buffer(NumberOfElements, BuffersCount)
		{
			_bufferType = GL_ARRAY_BUFFER;
			_bufferUsage = BufferUsage;
			_buffersSize *= sizeof(Vertex);
		}
	};

	class ElementArrayBuffer : public Buffer<GLuint*>
	{
	public:
		ElementArrayBuffer(GLushort BuffersCount = 1, GLenum BufferUsage = GL_STATIC_DRAW) : Buffer(BuffersCount)
		{
			_bufferType = GL_ELEMENT_ARRAY_BUFFER;
			_bufferUsage = BufferUsage;
			_buffersSize = sizeof(GLuint);
		}
		ElementArrayBuffer(GLsizei NumberOfElements, GLushort BuffersCount = 1, GLenum BufferUsage = GL_STATIC_DRAW) : Buffer(NumberOfElements, BuffersCount)
		{
			_bufferType = GL_ELEMENT_ARRAY_BUFFER;
			_bufferUsage = BufferUsage;
			_buffersSize *= sizeof(GLuint);
		}
	};
}