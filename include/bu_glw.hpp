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
#include "GL/gl3w.h"
#include "GL/gl.h"
#include "bu_glw_except.hpp"

/* Should constructors leave everything bound?
 * NOTE: Even if you define this as 0 the constructors which initialize buffers with data will bind the given buffer. VAOs do not bind in that case. The default constructors still won't bind anywhere. If set to 1 every constructor will automatically bind the created object.*/
#ifndef BU_GLW_CONSTRUCTORS_BIND 
#define BU_GLW_CONSTRUCTORS_BIND 1
#endif 

/************************** Shaders *************************/

class VertexShader{

};

class FragmentShader{

};

class ShaderProgram{

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
