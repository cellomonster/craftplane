#include "shader.h"

Shader::Shader() {
	ID = 0;
}

// constructor reads & builds shader
Shader::Shader(const char* vertPath, const char* fragPath) {
	// retrieve vert/frag shource from path
	std::string vertCodeStr, fragCodeStr;
	std::ifstream vertFile, fragFile;
	vertFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		// open files
		vertFile.open(vertPath);
		fragFile.open(fragPath);
		std::stringstream vertStream, fragStream;
		// read file's buffers into streams
		vertStream << vertFile.rdbuf();
		fragStream << fragFile.rdbuf();
		// close files
		vertFile.close();
		fragFile.close();
		// streams to strings
		vertCodeStr = vertStream.str();
		fragCodeStr = fragStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "Could not read shader file!" << std::endl;
	}
	const char* vertCode = vertCodeStr.c_str();
	const char* fragCode = fragCodeStr.c_str();

	// compile
	unsigned int vert, frag;
	int success;
	char infoLog[512];

	vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertCode, NULL);
	glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vert, 512, NULL, infoLog);
		std::cout << "Could not compile vert shader!\n" << infoLog << std::endl;
	};

	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragCode, NULL);
	glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(frag, 512, NULL, infoLog);
		std::cout << "Could not compile frag shader!\n" << infoLog << std::endl;
	};

	ID = glCreateProgram();
	glAttachShader(ID, vert);
	glAttachShader(ID, frag);
	glLinkProgram(ID);
	// print linking errors if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "Could not link shader!\n" << infoLog << std::endl;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
}

// getter
unsigned int Shader::getID() {
	return ID;
}

// shader setters
void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

// use/activate shader
void Shader::use() {
	glUseProgram(ID);
}