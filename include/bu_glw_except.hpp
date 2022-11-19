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

class BuGlwOutOfBounds: public std::exception {
	std::string what_message = "Slow down! No such index exists.";

public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class BuGlwRealBad : public std::exception {
	std::string what_message = "You shouldn't see this. If you do something really bad has happened. No idea what, tho.";

public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class BuGlwMemoryError: public std::exception {
	std::string what_message = "Have you run out of memory or what? How? (some memory operation prbably returned null)";

public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};
class GLShaderCompilationFailed : public std::exception {
	std::string what_message = "Failed to compile a shader.";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class GLShaderProgramLinkingFailed : public std::exception {
	std::string what_message = "Failed to link a shader program.";
public:
	const char* what() const noexcept override{
		return what_message.c_str();
	}
};

class GLInexistentUniform : public std::exception {
	std::string what_message = "No uniform could be retrieved with the name given.";
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
