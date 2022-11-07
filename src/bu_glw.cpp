/* Main header file for Benoe's Utilities: OpenGL wrappers
 *
 * For license see LICENSE.
 *
 * Project worked on by:
 * 2022 - present: Thomas Benoe */

#include "bu_glw.hpp"

/************************** Shaders *************************/

Shader::Shader(const char* path, GLenum type) : m_shader_type{type}{
	m_code = bu_glw_read_file_into_string(path);
};
Shader::~Shader(){
	free(m_code);
	glDeleteShader(m_ID);
}

void Shader::compile(){
	m_ID = glCreateShader(m_shader_type);
	glShaderSource(m_ID, 1, &m_code, NULL);
	free(m_code);
	m_code = NULL;
	glCompileShader(m_ID);
	int  success;
	char infoLog[512];
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(m_ID, 512, NULL, infoLog);
		fprintf(stderr, "Error during shader compilation: %s\n", infoLog);
		throw( GLShaderCompilationFailed() );
	}
}


char* bu_glw_read_file_into_string(const char* path){
	FILE* file = fopen(path, "r");
	
	if(file == NULL){
		throw(BuGlwBadFilePath());
	}

	char* string = (char*)calloc( 512, sizeof(char) );
	unsigned int allocated_mem = 512;
	unsigned int index = 0;

	
	char read = (char)fgetc(file);
	while( read != EOF){

		if(index < allocated_mem - 1){
			string[index] = read;
			index++;
		}else{
			char* new_buff = (char*)realloc(string, 2*allocated_mem);	
			allocated_mem *= 2;
			if(new_buff != nullptr){
				string = new_buff;
			}else{
				throw(std::bad_alloc());
			}
			string[allocated_mem - 1] = '\0';
			string[index] = read;
			index++;
		}

		read = (char)fgetc(file);
	}

#if BU_GLW_OPTIMIZE_MEMORY==1
	/* Resize to the minimum necessary size*/
	char* new_buff = (char*)realloc(string, 2*allocated_mem);	
	if(new_buff != nullptr){
		string = new_buff;
	}else{
		throw(std::bad_alloc());
	}
	string[index + 1] = '\0';
#endif

fclose(file);

return string;
}

/******************************** VBO *************************************/
VBO::VBO() : 
	m_ID{666}, /* An evil default number. It should be replaced either way, but if it isn't it should at least cause a nice crash and be visible in the debugger. */
	m_length{0},
	m_draw_mode{GL_STATIC_DRAW}
{
	glGenBuffers(1, &m_ID);
#if BU_GLW_CONSTRUCTORS_BIND==1 
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
#endif
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

void VBO::bind() const{
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void VBO::unbind() const{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::map(void (*f)(void*), GLenum mode) const{
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	void* ptr = glMapBuffer(GL_ARRAY_BUFFER, mode);
	f(ptr);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void VBO::map(void (*f)(void*)) const {
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
#if BU_GLW_CONSTRUCTORS_BIND==1 
	glBindVertexArray(m_ID);
#endif
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

/************************* EBO ******************************/

EBO::EBO() : 
	m_ID{666}, /* An evil default number. It should be replaced either way, but if it isn't it should at least cause a nice crash and be visible in the debugger. */
	m_length{0},
	m_draw_mode{GL_STATIC_DRAW}
{
	glGenBuffers(1, &m_ID);
#if BU_GLW_CONSTRUCTORS_BIND==1 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
#endif
}

EBO::EBO(const unsigned int* array, GLuint length, GLenum draw_mode) : 
	m_length{length},
	m_draw_mode{draw_mode}
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, length*sizeof(unsigned int), array, draw_mode);
}

EBO::~EBO(){
	glDeleteBuffers(1, &m_ID);
}

void EBO::data(unsigned int* data, GLuint length){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, length*sizeof(unsigned int), data, m_draw_mode);
}

void EBO::partial_data(GLintptr index, unsigned int* data, GLuint length){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, index, length*sizeof(unsigned int), data);
}

void EBO::bind(){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

void EBO::unbind(){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::map(void (*f)(void*), GLenum mode){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	void* ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, mode);
	f(ptr);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}
