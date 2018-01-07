#include <fstream>

#include "vGLSLShader.h"

namespace Vel
{
	using namespace std;

	Shader::Shader()
	{
		_totalShaders = 0;
		_shaders[VERTEX_SHADER] = false;
		_shaders[FRAGMENT_SHADER] = false;
		_shaders[GEOMETRY_SHADER] = false;
		_attributeList.clear();
		_uniformLocationList.clear();
	}

	Shader::~Shader()
	{
		_attributeList.clear();
		_uniformLocationList.clear();
		//glDeleteProgram(_program);
	}

	void Shader::LoadFromString(uint32_t Type, const string & Source)
	{
	}

	void Shader::LoadFromFile(uint32_t Shader, const string & Filename)
	{
	}

	void Shader::CreateAndLinkProgram()
	{
	}

	void Shader::Activate()
	{
		//glUseProgram(_program);
	}

	void Shader::Deactivate()
	{
		//glUseProgram(0);
	}

	void Shader::SetAttributes(const vector<string> &Attributes)
	{
		for (auto &Att : Attributes)
		{
			SetAttributes(Att);
		}
	}
	void Shader::SetUniforms(const vector<string>& Uniforms)
	{
		for (auto &Uni : Uniforms)
		{
			SetUniforms(Uni);
		}
	}
	void Shader::SetAttributes(const string & Attribute)
	{
		//auto att = Attribute.c_str();
		//_attributeList[Attribute] = glGetAttribLocation(_program, att);
	}

	void Shader::SetUniforms(const string & Uniform)
	{
		//auto uni = Uniform.c_str();
		//_uniformLocationList[Uniform] = glGetUniformLocation(_program, uni);
	}

	void Shader::DeleteShaderProgram()
	{
		//glDeleteProgram(_program);
	}
}