/* Main header file for Benoe's Utilities: OpenGL wrappers
 *
 * Minimalist, unopinionated C-style C++ wrappers for OpenGL which make it less verbose
 * and easier to use.
 *
 * For license see LICENSE.
 * 
 * Project worked on by:
 * 2022 - present: Thomas Benoe */
#ifndef BU_GLW_HEADER
#define BU_GLW_HEADER

#include <stdlib.h>
#include <stdio.h>
#include "GL/gl3w.h"
#include "GL/gl.h"
#include "bu_glw_except.hpp"

/* Should constructors leave everything bound?
 * NOTE: Even if you define this as 0 the constructors which initialize buffers with data will bind the given buffer. VAOs do not bind in that case. The default constructors still won't bind anywhere. If set to 1 every constructor will automatically bind the created object.*/
#ifndef BU_GLW_CONSTRUCTORS_BIND 
#define BU_GLW_CONSTRUCTORS_BIND 1
#endif 

/* Should some little memory optimization run at the cost of very little cpu? */
#ifndef BU_GLW_OPTIMIZE_MEMORY
#define BU_GLW_OPTIMIZE_MEMORY 1
#endif

/* Should bounds checks be done in some cases? This, though not recommended, may be defined to save some cpu cycles on those checks once you are sure your program has no out of bounds access.*/
#ifndef BU_GLW_NO_BOUNDS_CHECKING
#define BU_GLW_NO_BOUNDS_CHECKING 0
#endif


/*
 * Some #defines use these variables to decide how they should behave.
 * */
#ifndef OPENGL_VERSION_MAJOR
#define OPENGL_VERSION_MAJOR 4
#endif
#ifndef OPENGL_VERSION_MINOR
#define OPENGL_VERSION_MINOR 2
#endif

#ifndef BU_GLW_MAX_UNIFORM_NAME_LENGTH
#define BU_GLW_MAX_UNIFORM_NAME_LENGTH 32
#endif

/************************** Shaders *************************/

/* Forward declarations*/
class Shader;
class VertexShader;
class FragmentShader;
class ShaderProgram;
struct Uniform;

char* bu_glw_read_file_into_string(const char* path);

struct Uniform{
	char name[BU_GLW_MAX_UNIFORM_NAME_LENGTH + 1];
	GLint ID;
};

class Shader{
public:
	GLchar* m_code;
	const char* m_name;	
	GLuint m_ID;
	const GLenum m_shader_type;
	

protected:
public:
	Shader(const char* path, GLenum type);
	/* Move constructor. */
	Shader(Shader&&) = default;
	~Shader();

	/* No copy constructor. */
	Shader(const Shader&) = delete;
	/* No assignment operator. */
	Shader& operator=(const Shader&) = delete;

public:
	void compile(); /* May throw exceptions if any errors occur. */
	void attachTo(const GLuint program_id);
	

	/* Attention! If you use OpenGL version below 4.1 these will run glUseProgram on the vertex shader.
	 * In OpenGL versions 4.1 and above these will us glProgramUniform* to set the variables and thus won't bind any programs. */

	#if OPENGL_VERSION_MAJOR > 4 && OPENGL_VERSION_MINOR > 1	
	#endif


};


class VertexShader : public Shader{
	public:
	VertexShader(const char* path) : Shader(path, GL_VERTEX_SHADER){};
	friend ShaderProgram;
};


class FragmentShader : public Shader{
	public:
	FragmentShader(const char* path) : Shader(path, GL_FRAGMENT_SHADER){};
	friend ShaderProgram;
};

class ShaderProgram{
public:
	FragmentShader m_fs;
	VertexShader m_vs;
	const GLuint m_ID;

	Uniform* m_uniforms;
	unsigned int m_uniform_list_size;
	unsigned int m_uniform_list_length;
public:
	ShaderProgram(VertexShader& vertex_shader, FragmentShader& fragment_shader);
	ShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path);
	
	void use();

	unsigned int registerUniform(const char* name); /* Register a uniform for the current program. It will be assigned an ID automatically (this ID is the return value) and it will be looked up on the GPU. May throw if the uniform does not exist on the GPU.*/
	void finishUniformRegistration(); /* This optimizes the used memory to the minimum and expects you to not register new uniforms. */

	unsigned int findUniformID(); /* Query for the ID of a named uniform stored in this class. Beware! This uses string comparisons and is thus slow. You should not use this. You should retrieve the ID of uniforms when registering them. Attention! There is no type checking! You must ensure taht you pass the correct number and type of arguments to setUniform when setting uniforms. */

	void setUniform(unsigned int ID, GLfloat v0);
	void setUniform(unsigned int ID, GLfloat v0, GLfloat v1);
	void setUniform(unsigned int ID, GLfloat v0, GLfloat v1, GLfloat v2);
	void setUniform(unsigned int ID, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

	void setUniform(unsigned int ID, GLint v0);
	void setUniform(unsigned int ID, GLint v0, GLint v1);
	void setUniform(unsigned int ID, GLint v0, GLint v1, GLint v2);
	void setUniform(unsigned int ID, GLint v0, GLint v1, GLint v2, GLint v3);

	void setUniform(unsigned int ID, GLuint v0);
	void setUniform(unsigned int ID, GLuint v0, GLuint v1);
	void setUniform(unsigned int ID, GLuint v0, GLuint v1, GLuint v2);
	void setUniform(unsigned int ID, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
};

/*************************** VBO ****************************/

class VBO{
	GLuint m_ID;
	GLuint m_draw_mode;
	unsigned int m_length;
public:
	VBO();
	VBO(const float* array, GLuint length, GLenum draw_mode);
	VBO(const float* array, GLuint length) : VBO(array, length, GL_STATIC_DRAW){};
	
	/* Convert constructor from array. */
	template<GLuint L>
	VBO(const float (*array)[L]) : VBO((float*)array, L){};

	~VBO();
	/* No copy constructor and assignment operator - We don't want multiple instances corresponding to one buffer on the GPU, because that could lead to some spaghetti code.*/
	VBO(VBO&) = delete;
	VBO operator=(const VBO&) = delete;

	void bind() const;
	void unbind() const;
	void data(float* data, GLuint length);
	void partial_data(GLintptr index, float* data, GLuint length);
	
	/* Map the buffer and run the function f on the resulting array. */
	void map(void (*f)(void* buffer), GLenum mode) const;
	void map(void (*f)(void* buffer)) const;
};

struct VertexAttrib{
	unsigned int num_fields;
	GLenum field_type;
	size_t field_size;
	GLboolean normalized;
};

class VAO{

	GLuint m_ID;
	VertexAttrib* m_attributes;
	unsigned int m_num_attributes;
	unsigned int m_num_allocated_attributes;
	GLsizei m_stride;
public:

	VAO();
	~VAO();
	
	void bind();
	void unbind();
	void add_attribute(VertexAttrib atr); /* Add an attribute cpu-side */ 
	void add_attribute(uint num_fields, GLenum field_type = GL_FLOAT, size_t field_size = sizeof(unsigned int), GLboolean normalized = GL_FALSE); /* Add an attribute cpu-side */ 

	void bind_attributes(); /* Push the attributes to the gpu and free them on the cpu-side. */	
	void bind_attributes_no_discard();	/*Push the attributes to the gpu but also keep them around cpu-side. */
};

/************************* EBO ******************************/

class EBO{
	GLuint m_ID;
	GLuint m_draw_mode;
	unsigned int m_length;
public:
	EBO();
	EBO(const unsigned int* array, GLuint length, GLenum draw_mode);
	EBO(const unsigned int* array, GLuint length) : EBO(array, length, GL_STATIC_DRAW){};
	
	/* Convert constructor from array. */
	template<GLuint L>
	EBO(const unsigned int (*array)[L]) : EBO((unsigned int*)array, L){};

	~EBO();
	/* No copy constructor and assignment operator - We don't want multiple instances corresponding to one buffer on the GPU, because that could lead to some spaghetti code.*/
	EBO(EBO&) = delete;
	EBO operator=(const EBO&) = delete;

	void bind();
	void unbind();
	void data(unsigned int* data, GLuint length);
	void partial_data(GLintptr index, unsigned int* data, GLuint length);
	
	/* Map the buffer and run the function f on the resulting array. */
	void map(void (*f)(void* buffer), GLenum mode=GL_READ_WRITE);
};
#endif
