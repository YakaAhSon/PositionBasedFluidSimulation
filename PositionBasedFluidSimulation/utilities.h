#pragma once
#ifndef __UTILITIES__
#define __UTILITIES__
#include<string>
#include<glad/glad.h>


#ifdef _DEBUG
#define DEBUG_LOG printf
#else
#define DEBUG_LOG(x)
#endif

namespace util {
    GLuint createShader(const char* source, GLenum type, const char* errinfo = "");
    void linkProgram(GLuint program, const char* errinfo = "");

    // copute shader
    GLuint createProgram_C(const std::string& source);
    
    // vertex+fragment shader
    GLuint createProgram_VF(const std::string& v_source, const std::string& f_source);

    std::string readFile(const std::string& file_name);

    class ComputeShader {
    protected:
        void createProgram(const std::string& file_name) { program = createProgram_C(readFile(file_name)); }
    public:
        GLuint program;
        virtual void initialize()=0;
        void use() { glUseProgram(program); };
    };
}

#endif