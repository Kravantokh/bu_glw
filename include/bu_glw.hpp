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

	void bind();
	void unbind();
	void data(float* data, GLuint length);
	void partial_data(GLintptr index, float* data, GLuint length);
	
	/* Map the buffer and run the function f on the resulting array. */
	void map(void (*f)(void* buffer), GLenum mode);
	void map(void (*f)(void* buffer));
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
	void add_attribute(uint num_fields, GLenum field_type = GL_FLOAT, size_t field_size = sizeof(float), GLboolean normalized = GL_FALSE); /* Add an attribute cpu-side */ 

	void bind_attributes(); /* Push the attributes to the gpu and free them on the cpu-side. */	
	void bind_attributes_no_discard();	/*Push the attributes to the gpu but also keep them around cpu-side. */
};

#endif
