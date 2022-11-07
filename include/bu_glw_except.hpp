#ifndef BU_GLW_EXCEPTION_HEADER
#define BU_GLW_EXCEPTION_HEADER

#include <stdio.h>
#include <exception>
#include <string>

class BuGlwBadFilePath : public std::exception {
	std::string what_message = "A bad file path was specified. The file does not exist or could not be opened.";

public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class GLShaderCompilationFailed : public std::exception {
	std::string what_message = "An OpenGL function returned null when it was not supposed to.";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class GLNullPointerReturned : public std::exception {
	std::string what_message = "An OpenGL function returned null when it was not supposed to.";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};


#endif
