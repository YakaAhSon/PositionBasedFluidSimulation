#include"utilities.h"

using namespace std;

using namespace util;

GLuint util::createShader(const char* source, GLenum type, const char* errinfo) {
    int len = std::strlen(source);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &len);
    glCompileShader(shader);
    GLint testVal = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &testVal);
    if (testVal == GL_FALSE)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        glDeleteShader(shader);
        DEBUG_LOG("shader error (%s) : %s\n", errinfo, infoLog);
        return 0;
    }


    return shader;
}



void util::linkProgram(GLuint program, const char* errinfo) {
    GLint testVal;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &testVal);
    if (testVal == GL_FALSE) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        glDeleteProgram(program);

        DEBUG_LOG("link error (%s) : %s\n", errinfo, infoLog);
    }
}

GLuint util::createProgram_C(std::string source) {
    GLuint program = glCreateProgram();
    GLuint shader = createShader(source.c_str(), GL_COMPUTE_SHADER);

    glAttachShader(program, shader);
    linkProgram(program);

    return program;
}

GLuint util::createProgram_VF(std::string v_source, std::string f_source) {
    GLuint program = glCreateProgram();
    GLuint v_shader = createShader(v_source.c_str(), GL_VERTEX_SHADER);
    GLuint f_shader = createShader(f_source.c_str(), GL_FRAGMENT_SHADER);

    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);

    linkProgram(program);

    return program;
}