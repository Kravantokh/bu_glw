/* Main header file for Benoe's Utilities: OpenGL wrappers
 *
 * For license see LICENSE.
 *
 * Project worked on by:
 * 2022 - present: Thomas Benoe */

#include "bu_glw.hpp"
#include "GL/gl.h"

/******************************** VBO *************************************/
VBO::VBO() : 
	m_ID{666}, /* An evil default number. It should be replaced either way, but if it isn't it should at least cause a nice crash and be visible in the debugger. */
	m_length{0},
	m_draw_mode{GL_STATIC_DRAW}
{
	glGenBuffers(1, &m_ID);
}

VBO::VBO(const float* array, GLuint length, GLenum draw_mode) : 
	m_length{length},
	m_draw_mode{draw_mode}
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, length*sizeof(float), array, draw_mode);
}

VBO::~VBO(){
	glDeleteBuffers(1, &m_ID);
}

void VBO::data(float* data, GLuint length){
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, length*sizeof(float), data, m_draw_mode);
}

void VBO::partial_data(GLintptr index, float* data, GLuint length){
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferSubData(GL_ARRAY_BUFFER, index, length*sizeof(float), data);
}

void VBO::bind(){
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void VBO::unbind(){
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::map(void (*f)(void*), GLenum mode){
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	void* ptr = glMapBuffer(GL_ARRAY_BUFFER, mode);
	f(ptr);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void VBO::map(void (*f)(void*)){
	map(f, GL_READ_WRITE);
}

/******************************** VAO ****************************************/

VAO::VAO() :
	m_ID{666},
	m_attributes{nullptr},
	m_num_attributes{0},
	m_num_allocated_attributes{0},
	m_stride{0}
{
	glGenVertexArrays(1, &m_ID);
}

VAO::~VAO(){
	free(m_attributes);
	glDeleteVertexArrays(1, &m_ID);
}


void VAO::bind(){
	glBindVertexArray(m_ID);
}

void VAO::unbind(){
	glBindVertexArray(0);
}


void VAO::add_attribute(VertexAttrib atr){
	/* No reallocation or initialization needed. Should be the most frequent case.*/
	if(m_num_attributes <= m_num_allocated_attributes && m_num_attributes != 0){
		m_attributes[m_num_allocated_attributes] = atr;
		m_num_attributes++;
		m_stride += atr.field_size * atr.num_fields;
		return;
	}
	
	/* The container should grow. A rarer case.*/
	if(m_num_attributes > m_num_allocated_attributes){
		m_attributes = (VertexAttrib*)realloc(m_attributes, 2*m_num_allocated_attributes);
		m_num_allocated_attributes *= 2;
		m_num_attributes++;
		m_stride += atr.field_size * atr.num_fields;
		return;
	}

	/* Initialization. Should be the rarest. It should happen only on object creation.*/
	if(m_attributes == nullptr){
		m_attributes = (VertexAttrib*)malloc( sizeof(VertexAttrib) * 2) /* Magic number */;
		m_num_allocated_attributes = 2;
		m_num_attributes = 1;
		m_attributes[0] = atr;
		m_stride += atr.field_size * atr.num_fields;
		return;
	}

}

void VAO::add_attribute(uint num_fields, GLenum field_type, size_t field_size, GLboolean normalized){ /* Add an attribute cpu-side */ 
	add_attribute( (VertexAttrib){num_fields, field_type, field_size, normalized} );
}

void VAO::bind_attributes_no_discard(){
	size_t offset = 0;
	for(int i = 0; i < m_num_attributes; ++i){
		glVertexAttribPointer(
				i,
				m_attributes[i].num_fields,
				m_attributes[i].field_type,
				m_attributes->normalized,
				m_stride,
				(void*)(offset)
			);
		offset += m_attributes[i].field_size*m_attributes[i].num_fields;
		glEnableVertexAttribArray(i);
	}
}

void VAO::bind_attributes(){
	bind_attributes_no_discard();
	free(m_attributes);
	m_attributes = nullptr;
}
