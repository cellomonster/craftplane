#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
private:
	// program ID
	unsigned int ID;

public:
	// constructors
	Shader();
	Shader(const char* vertPath, const char* fragPath);

	// getters
	unsigned int getID();

	// shader setters
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;

	// use/activate shader
	void use();
};