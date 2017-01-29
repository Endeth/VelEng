
#include "VGLSLShader.h"
#include <fstream>

namespace Vel
{
	using namespace std;

	GLSLShader::GLSLShader()
	{
		_totalShaders = 0;
		_shaders[VERTEX_SHADER] = 0;
		_shaders[FRAGMENT_SHADER] = 0;
		_shaders[GEOMETRY_SHADER] = 0;
		_attributeList.clear();
		_uniformLocationList.clear();
	}



	GLSLShader::~GLSLShader()
	{
		_attributeList.clear();
		_uniformLocationList.clear();
		glDeleteProgram(_program);
	}

	void GLSLShader::LoadFromString(GLenum Type, const string & Source)
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

	void GLSLShader::LoadFromFile(GLenum Shader, const string & Filename)
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

	void GLSLShader::CreateAndLinkProgram()
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

	void GLSLShader::Activate()
	{
		glUseProgram(_program);
	}

	void GLSLShader::Deactivate()
	{
		glUseProgram(0);
	}

	void GLSLShader::SetAttributes(const vector<string> &Attributes)
	{
		for (auto &Att : Attributes)
		{
			SetAttributes(Att);
		}
	}
	void GLSLShader::SetUniforms(const vector<string>& Uniforms)
	{
		for (auto &Uni : Uniforms)
		{
			SetUniforms(Uni);
		}
	}
	void GLSLShader::SetAttributes(const string & Attribute)
	{
		auto att = Attribute.c_str();
		_attributeList[Attribute] = glGetAttribLocation(_program, att);
	}

	void GLSLShader::SetUniforms(const string & Uniform)
	{
		auto uni = Uniform.c_str();
		_uniformLocationList[Uniform] = glGetUniformLocation(_program, uni);
	}

	GLuint GLSLShader::GetAttribute(const string& Attribute)
	{
		return _attributeList[Attribute];
	}

	GLuint GLSLShader::GetUniform(const string& Uniform)
	{
		return _uniformLocationList[Uniform];
	}

	void GLSLShader::DeleteShaderProgram()
	{
		glDeleteProgram(_program);
	}
}