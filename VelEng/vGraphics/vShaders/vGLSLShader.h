#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "external/glm/glm.hpp"
#include "external/glm/gtc/type_ptr.hpp"

namespace Vel
{
	template <typename UniformTypePointer>
	struct Uniform
	{
		Uniform(const std::string& uniformName, UniformTypePointer value, uint32_t count = 1) : UniformName(uniformName), Value(value), Count(count)
		{
		}
		std::string UniformName;
		uint32_t Count;
		UniformTypePointer Value;
		using UniformType = UniformTypePointer;
	};
	template <>
	struct Uniform<glm::mat4>
	{
		Uniform(const std::string& uniformName, const glm::mat4 &value, uint32_t count = 1, bool transpose = false) : UniformName(uniformName), Value(value), Count(count), Transpose(transpose)
		{
		}
		std::string UniformName;
		uint32_t Count;
		glm::mat4 Value;
		bool Transpose;
		using UniformType = glm::mat4;
	};
	//TODO rewrite this
	class Shader
	{
	public:
		Shader();
		~Shader();
		void LoadFromString(uint32_t Type, const std::string& Source);
		void LoadFromFile(uint32_t Shader, const std::string& Filename);
		void CreateAndLinkProgram();

		void Activate();
		void Deactivate();

		void SetAttributes(const std::vector<std::string>& Attributes);
		void SetUniforms(const std::vector<std::string>& Uniforms);
		void SetAttributes(const std::string& Attribute);
		void SetUniforms(const std::string& Uniform);

		uint32_t GetProgramID() { return _program; }
		void DeleteShaderProgram();

		template <typename T, typename... UniVal>
		void SetUniformsValue(Uniform<T> UniformVal, UniVal... Rest)
		{
			SetUniformValue(UniformVal);
			SetUniformsValue(Rest...);
		}
		template <typename T>
		void SetUniformsValue(Uniform<T> UniformVal)
		{
			SetUniformValue(UniformVal);
		}

	private:
		void SetUniformValue(Uniform<float>& V)
		{
		}
		void SetUniformValue(Uniform<int>& V)
		{
		}
		void SetUniformValue(Uniform<glm::vec3>& V)
		{
		}
		void SetUniformValue(Uniform<glm::mat4>& V)
		{
		}

		enum ShaderType
		{
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			GEOMETRY_SHADER
		};
		uint32_t _program;
		int _totalShaders;
		bool _shaders[3];
		std::map<std::string, uint16_t> _attributeList;
		std::map<std::string, uint16_t> _uniformLocationList;
	};
}