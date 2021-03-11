#pragma once
#include<string>
class Shader {
	int ReadShaderFile(GLint shader, std::string filename);
	void CompileInfoPrint(GLint shadrObject);
	void LinkInfoPrint(GLint shadrObject);
public:
	GLint LoadShader(const char* VertSource, const char* FragSource, GLuint position, std::string name);
};