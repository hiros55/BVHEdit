#include "glew.h"
#include <fstream>
#include<string>
#include<iostream>
#include "Shader.hpp"

//�V�F�[�_�[�e�L�X�g�ǂݍ���&�֘A�t��
int Shader::ReadShaderFile(GLint shader, std::string filename) {
	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.is_open() == 0) {
		std::cerr << "Could not open file:" << filename << std::endl;
		return -1;
	}

	std::string source;
	std::string line;

	//�ŏI�s�܂ŃV�F�[�_�[�̃e�L�X�g�ǂݍ���
	while (std::getline(file, line)) {
		source += line + "\n";
	}

	if (source.empty()) {
		std::cerr << "Could not read file:" << filename << std::endl;
		return -1;
	}

	const GLchar* sourcePtr = (const GLchar*)source.c_str();
	GLint length = source.length();
	glShaderSource(shader, 1, &sourcePtr, &length);
	return 0;
}

//�R���p�C���G���[�p �W���G���[�o��
void Shader::CompileInfoPrint(GLint shadrObject) {
	GLint infoLogLength;
	glGetShaderiv(shadrObject, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 1) {
		GLchar* infoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(shadrObject, infoLogLength, NULL, infoLog);
		std::cerr << "InfoLog:" << infoLog << std::endl;
		delete[] infoLog;
	}
	else {
		std::cerr << "Compile Error:Could not catch shadre log" << std::endl;
	}
}

//�����N�G���[�p �W���G���[�o��
void Shader::LinkInfoPrint(GLint program) {
	GLint infoLogLength;
	glGetShaderiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 1) {
		GLchar* infoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(program, infoLogLength, NULL, infoLog);
		std::cerr << "InfoLog:" << infoLog << std::endl;
	}
	else {
		std::cerr << "Link Error:Could not catch shadre log" << std::endl;
	}
}

//�V�F�[�_�[�ǂݍ���(�R���p�C���E�����N)
GLint Shader::LoadShader(const char* VertSource, const char* FragSource, GLuint position, std::string name) {

	GLint VShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLint FShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	if (ReadShaderFile(VShaderId, VertSource)) return 0;
	if (ReadShaderFile(FShaderId, FragSource)) return 0;

	GLint compileStatus;

	glCompileShader(VShaderId);
	glGetShaderiv(VShaderId, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		CompileInfoPrint(VShaderId);
		exit(1);
	}

	glCompileShader(FShaderId);
	glGetShaderiv(FShaderId, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		CompileInfoPrint(FShaderId);
		exit(1);
	}

	GLint Program = glCreateProgram();

	glAttachShader(Program, VShaderId);
	glAttachShader(Program, FShaderId);

	glDeleteShader(VShaderId);
	glDeleteShader(FShaderId);

	glBindAttribLocation(Program, position, name.c_str());

	glLinkProgram(Program);
	glGetShaderiv(Program, GL_LINK_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		LinkInfoPrint(Program);
		exit(1);
	}
	return Program;
}