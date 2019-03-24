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
    GLuint createProgram_C(std::string source);
    
    // vertex+fragment shader
    GLuint createProgram_VF(std::string v_source, std::string f_source);
}

#endif