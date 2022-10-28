#ifndef BU_GLW_EXCEPTION_HEADER
#define BU_GLW_EXCEPTION_HEADER

#include <stdio.h>
#include <exception>
#include <string>

class GLNullPointerReturned : public std::exception {
	std::string what_message = "An OpenGL function returned null when it was not supposed to.";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class GL : public std::exception {
	std::string what_message = "";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class GLTemplateException : public std::exception {
	std::string what_message = "";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};
#endif
