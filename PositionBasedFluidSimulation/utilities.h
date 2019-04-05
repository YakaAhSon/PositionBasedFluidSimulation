#pragma once
#ifndef __UTILITIES__

#define __UTILITIES__

#include<string>
#include<glad/glad.h>
#include<chrono>
#include<string>
#include<iostream>
#include<numeric>
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

    class Timer {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> _start_;
    public:
        Timer() { tic(); }
        void tic() { 
            _start_ = std::chrono::high_resolution_clock::now(); 
        }
        double toc(const std::string& info = "") {
            using namespace std::chrono;
            auto end = std::chrono::high_resolution_clock::now();

            duration<double> time_span = duration_cast<duration<double>>(end - _start_);;

            _start_.time_since_epoch();
            if (info != "") {
                std::cout << info << " :\t" << static_cast<int>(time_span.count()*1000000) << " Microsecond"<<std::endl;
            }
            return static_cast<int>(time_span.count() * 1000000);
        }
    };
}

#endif