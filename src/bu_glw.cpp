/* Main header file for Benoe's Utilities: OpenGL wrappers
 *
 * For license see LICENSE.
 *
 * Project worked on by:
 * 2022 - present: Thomas Benoe */

#include "bu_glw.hpp"
#include <string.h>
#ifdef __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#else
#include <stdio.h>
#endif

/************************** Shaders *************************/

Shader::Shader(const char* path, GLenum type) : 
	m_code{nullptr},
	m_shader_type{type}
{
	if(path==nullptr){
		return;
	}
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
	char message[512];
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(m_ID, 512, NULL, message);
		fprintf(stderr, "Error during shader compilation: %s\n", message);
		throw( GLShaderCompilationFailed() );
	}
}


void Shader::attachTo(const GLuint prog){
	glAttachShader(prog, m_ID);
}

void setUniform(const char* name, GLfloat v0, GLfloat v1);
void setUniform(const char* name, GLfloat v0, GLfloat v1, GLfloat v2);
void setUniform(const char* name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);


ShaderProgram::ShaderProgram(VertexShader& vs, FragmentShader& fs) :
	m_gs{nullptr},
	m_vs{std::move(vs)},
	m_fs{std::move(fs)},
	m_uniform_list_length{0},
	m_uniform_list_size{0},
	m_uniforms{nullptr},
	m_ID{glCreateProgram()}
{
	vs.attachTo(m_ID);
	fs.attachTo(m_ID);
	glLinkProgram(m_ID);
	int  success = 0;
	char message[512] = {0};
	glGetProgramiv(m_ID, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(m_ID, 512, NULL, message);
		fprintf(stderr, "Error during shader linking: %s\n", message);
		throw( GLShaderProgramLinkingFailed() );
	}
}

ShaderProgram::ShaderProgram(const char* vs_path, const char* fs_path) : 
	m_vs{vs_path},
	m_gs{nullptr},
	m_fs{fs_path},
	m_uniform_list_length{0},
	m_uniform_list_size{0},
	m_uniforms{nullptr},
	m_ID{glCreateProgram()}
{
	m_vs.compile();
	m_fs.compile();
	
	m_vs.attachTo(m_ID);
	m_fs.attachTo(m_ID);
	glLinkProgram(m_ID);
	int  success = 0;
	char message[512] = {0};
	glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(m_ID, 512, NULL, message);
		fprintf(stderr, "Error during shader linking: %s\n", message);
		throw( GLShaderProgramLinkingFailed() );
	}
}

ShaderProgram::ShaderProgram(const char* vs, const char* gs, const char* fs) :
	m_vs{vs},
	m_gs{gs},
	m_fs{fs},
	m_uniform_list_length{0},
	m_uniform_list_size{0},
	m_uniforms{nullptr},
	m_ID{glCreateProgram()}
{
	m_vs.compile();
	m_fs.compile();
	m_gs.compile();
	
	m_vs.attachTo(m_ID);
	m_fs.attachTo(m_ID);
	m_gs.attachTo(m_ID);
	glLinkProgram(m_ID);
	int  success = 0;
	char message[512] = {0};
	glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(m_ID, 512, NULL, message);
		fprintf(stderr, "Error during shader linking: %s\n", message);
		throw( GLShaderProgramLinkingFailed() );
	}
}



void ShaderProgram::use(){
	glUseProgram(m_ID);
}

unsigned int ShaderProgram::registerUniform(const char* name){
	/* Most frequent case: no need to resize.*/
	if(m_uniform_list_length < m_uniform_list_size){
		GLint location = glGetUniformLocation(m_ID, name);
		if(location == -1)
			throw(new GLInexistentUniform);
		Uniform* new_uniform = &m_uniforms[m_uniform_list_length];
		strncpy(&new_uniform->name[0], name, BU_GLW_MAX_UNIFORM_NAME_LENGTH);
		new_uniform->name[BU_GLW_MAX_UNIFORM_NAME_LENGTH] = '\0';
		new_uniform->ID = location;
		m_uniform_list_length++;
	}else{
		/* If not enough memory is available we shall allocate it.*/
		if(m_uniform_list_length == m_uniform_list_size){
			if(m_uniforms == nullptr){
				m_uniforms = (Uniform*)calloc( 4, sizeof(Uniform) );
				m_uniform_list_size = 4;
				GLint location = glGetUniformLocation(m_ID, name);
				if(location == -1)
					throw(GLInexistentUniform());
				Uniform* new_uniform = &m_uniforms[m_uniform_list_length];
				strncpy( &new_uniform->name[0], name, BU_GLW_MAX_UNIFORM_NAME_LENGTH);
				new_uniform->name[BU_GLW_MAX_UNIFORM_NAME_LENGTH] = '\0';
				new_uniform->ID = location;
				m_uniform_list_length++;
				if(m_uniforms == nullptr)
					throw(new BuGlwMemoryError);
			}else{
				Uniform* ptr = (Uniform*)realloc(m_uniforms, 2*m_uniform_list_size*sizeof(Uniform) );
				if(ptr == nullptr)
					throw(new BuGlwMemoryError);
				else{
					m_uniforms = ptr;
					m_uniform_list_size *= 2;
					m_uniform_list_length++;
				}
			}
		}else{
			/* No other case should exist. Any other state is invalid and may corrupt the program.*/
			throw(new BuGlwRealBad);
		}
	}
	return m_uniform_list_length;
}
void ShaderProgram::finishUniformRegistration(){
	Uniform* ptr = (Uniform*)realloc(m_uniforms, m_uniform_list_length * sizeof(Uniform) );
	if(ptr == nullptr)
		throw(new BuGlwMemoryError);
	else{
		m_uniforms = ptr;
		m_uniform_list_length = m_uniform_list_size;
	}

}

/* A macro to save me a bunch of typing the same code*/
#if !BU_GLW_NO_BOUNDS_CHECKING
	#ifdef BU_GLW_LOCAL_BOUNDS_CHECK
		#error "Why is this macro defined? It shouldn't ever be!"
	#else 
		#define BU_GLW_LOCAL_BOUNDS_CHECK \
		if(ID >= m_uniform_list_length || ID < 0)\
			throw(new BuGlwOutOfBounds);
	#endif
#else
	#define BU_GLW_LOCAL_BOUNDS_CHECK 
#endif

void ShaderProgram::setUniform(unsigned int ID, GLfloat v0){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform1f(m_uniforms[ID].ID, v0);

}
void ShaderProgram::setUniform(unsigned int ID, GLfloat v0, GLfloat v1){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform2f(m_uniforms[ID].ID, v0, v1);

}

void ShaderProgram::setUniform(unsigned int ID, GLfloat v0, GLfloat v1, GLfloat v2){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform3f(m_uniforms[ID].ID, v0, v1, v2);

}

void ShaderProgram::setUniform(unsigned int ID, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform4f(m_uniforms[ID].ID, v0, v1, v2, v3);
}


void ShaderProgram::setUniform(unsigned int ID, GLint v0){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform1i(m_uniforms[ID].ID, v0);
}
void ShaderProgram::setUniform(unsigned int ID, GLint v0, GLint v1){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform2i(m_uniforms[ID].ID, v0, v1);
}
void ShaderProgram::setUniform(unsigned int ID, GLint v0, GLint v1, GLint v2){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform3i(m_uniforms[ID].ID, v0, v1, v2);
}

void ShaderProgram::setUniform(unsigned int ID, GLint v0, GLint v1, GLint v2, GLint v3){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform4i(m_uniforms[ID].ID, v0, v1, v2, v3);
}



void ShaderProgram::setUniform(unsigned int ID, GLuint v0){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform1ui(m_uniforms[ID].ID, v0);
}

void ShaderProgram::setUniform(unsigned int ID, GLuint v0, GLuint v1){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform2ui(m_uniforms[ID].ID, v0, v1);
}

void ShaderProgram::setUniform(unsigned int ID, GLuint v0, GLuint v1, GLuint v2){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform3ui(m_uniforms[ID].ID, v0, v1, v2);
}

void ShaderProgram::setUniform(unsigned int ID, GLuint v0, GLuint v1, GLuint v2, GLuint v3){
	BU_GLW_LOCAL_BOUNDS_CHECK
	glUniform4ui(m_uniforms[ID].ID, v0, v1, v2, v3);
}

#undef BU_GLW_LOCAL_BOUNDS_CHECK


char* bu_glw_read_file_into_string(const char* path){
#if __linux__
/* Elegant mmap code for Linux */
	char* string;
	/* Check if file exists  */
	if (access(path, R_OK) == 0) {
		/* The file exists and can be accessed. Everything may proceed normally. */
		struct stat file_info;
		if( stat(path, &file_info) != 0 )
			throw( BuGlwIOError() ); /* stat didn't work. Too bad! */	

		/* Open file */
		int fd = open(path, O_RDONLY);
		if(fd == -1)
			throw( BuGlwIOError() ); /* open didn't work. Too bad! */	

		/* Memory map file */
		char* mapped_file = (char*)mmap(NULL , file_info.st_size, PROT_READ, MAP_SHARED,
                  fd, 0);
		if( mapped_file == (void*)-1 ){
			throw( BuGlwIOError() ); /* mmap didn't work. Catch this! */	
		}
		
		string = (char*)malloc( file_info.st_size + 1 ); /* Allocate buffer */
		if( string == NULL )
			throw(BuGlwMemoryError());/* Not enough memory. Too bad! */

		string[ file_info.st_size ] = 0; /* Null termination */
		memcpy(string, mapped_file, file_info.st_size);
		munmap(mapped_file, file_info.st_size);
	} else {
		/* File does not exist. Too bad! */
		throw(BuGlwBadFilePath());
	}

#else
	/* Possibly buggy code for the rest. */
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

#endif

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
