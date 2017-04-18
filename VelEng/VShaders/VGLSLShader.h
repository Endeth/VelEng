#pragma once

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include "../VOpenGL/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace Vel
{
	template <typename UniformTypePointer>
	struct Uniform
	{
		Uniform(const std::string& uniformName, UniformTypePointer value, GLuint count = 1) : UniformName(uniformName), Value(value), Count(count)
		{
		}
		std::string UniformName;
		GLuint Count;
		UniformTypePointer Value;
		using UniformType = UniformTypePointer;
	};
	template <>
	struct Uniform<glm::mat4>
	{
		Uniform(const std::string& uniformName, const glm::mat4 &value, GLuint count = 1, GLboolean transpose = GL_FALSE) : UniformName(uniformName), Value(value), Count(count), Transpose(transpose)
		{
		}
		std::string UniformName;
		GLuint Count;
		glm::mat4 Value;
		GLboolean Transpose;
		using UniformType = glm::mat4;
	};
	//TODO rewrite this
	class Shader
	{
	public:
		Shader();
		~Shader();
		void LoadFromString(GLenum Type, const std::string& Source);
		void LoadFromFile(GLenum Shader, const std::string& Filename);
		void CreateAndLinkProgram();

		void Activate();
		void Deactivate();

		void SetAttributes(const std::vector<std::string>& Attributes);
		void SetUniforms(const std::vector<std::string>& Uniforms);
		void SetAttributes(const std::string& Attribute);
		void SetUniforms(const std::string& Uniform);

		GLint GetAttribute(const std::string& Attribute); //TODO constexpr?
		GLint GetUniform(const std::string& Uniform);
		GLuint GetProgramID() { return _program; }
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
		void SetUniformValue(Uniform<GLfloat>& V)
		{
			glProgramUniform1f(_program, GetUniform(V.UniformName), V.Value);
		}
		void SetUniformValue(Uniform<int>& V)
		{
			glProgramUniform1i(_program, GetUniform(V.UniformName), V.Value);
		}
		void SetUniformValue(Uniform<glm::vec3>& V)
		{
			glProgramUniform3f(_program, GetUniform(V.UniformName), V.Value.x, V.Value.y, V.Value.z);
		}
		void SetUniformValue(Uniform<glm::mat4>& V)
		{
			glProgramUniformMatrix4fv(_program, GetUniform(V.UniformName), V.Count, V.Transpose, glm::value_ptr(V.Value));
		}

		enum ShaderType
		{
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			GEOMETRY_SHADER
		};
		GLuint _program;
		int _totalShaders;
		GLuint _shaders[3];
		std::map<std::string, GLint> _attributeList;
		std::map<std::string, GLint> _uniformLocationList;
	};
}