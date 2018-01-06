#include <fstream>

#include "vGLSLShader.h"

namespace Vel
{
	using namespace std;

	Shader::Shader()
	{
		_totalShaders = 0;
		_shaders[VERTEX_SHADER] = 0;
		_shaders[FRAGMENT_SHADER] = 0;
		_shaders[GEOMETRY_SHADER] = 0;
		_attributeList.clear();
		_uniformLocationList.clear();
	}

	Shader::~Shader()
	{
		_attributeList.clear();
		_uniformLocationList.clear();
		glDeleteProgram(_program);
	}

	void Shader::LoadFromString(GLenum Type, const string & Source)
	{
		GLuint shader = glCreateShader(Type);

		const char* ptmp = Source.c_str();
		glShaderSource(shader, 1, &ptmp, NULL);

		GLint status;
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			GLchar *infoLog = new GLchar[infoLogLength];
			glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
			cerr << "Compile log: " << infoLog << endl;
			delete[] infoLog;
		}
		_shaders[_totalShaders++] = shader;
	}

	void Shader::LoadFromFile(GLenum Shader, const string & Filename)
	{
		ifstream fp;
		fp.open(Filename.c_str(), ios_base::in);
		if (fp)
		{
			string line, buffer;
			while (getline(fp, line))
			{
				buffer.append(line);
				buffer.append("\r\n");
			}
			LoadFromString(Shader, buffer);
		}
		else
			cerr << "Error loading shader: " << Filename << endl;
	}

	void Shader::CreateAndLinkProgram()
	{
		_program = glCreateProgram();
		if (_shaders[VERTEX_SHADER] != 0)
		{
			glAttachShader(_program, _shaders[VERTEX_SHADER]);
		}
		if (_shaders[FRAGMENT_SHADER] != 0)
		{
			glAttachShader(_program, _shaders[FRAGMENT_SHADER]);
		}
		if (_shaders[GEOMETRY_SHADER] != 0)
		{
			glAttachShader(_program, _shaders[GEOMETRY_SHADER]);
		}

		GLint status;
		glLinkProgram(_program);
		glGetProgramiv(_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint infoLogLength;
			glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
			GLchar *infoLog = new GLchar[infoLogLength];
			glGetProgramInfoLog(_program, infoLogLength, NULL, infoLog);
			cerr << "Link log: " << infoLog << endl;
			delete[] infoLog;
		}

		glDeleteShader(_shaders[VERTEX_SHADER]);
		glDeleteShader(_shaders[FRAGMENT_SHADER]);
		glDeleteShader(_shaders[GEOMETRY_SHADER]);
	}

	void Shader::Activate()
	{
		glUseProgram(_program);
	}

	void Shader::Deactivate()
	{
		glUseProgram(0);
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
		auto att = Attribute.c_str();
		_attributeList[Attribute] = glGetAttribLocation(_program, att);
	}

	void Shader::SetUniforms(const string & Uniform)
	{
		auto uni = Uniform.c_str();
		_uniformLocationList[Uniform] = glGetUniformLocation(_program, uni);
	}

	GLint Shader::GetAttribute(const string& Attribute)
	{
		return _attributeList[Attribute];
	}

	GLint Shader::GetUniform(const string& Uniform)
	{
		return _uniformLocationList[Uniform];
	}

	void Shader::DeleteShaderProgram()
	{
		glDeleteProgram(_program);
	}
}