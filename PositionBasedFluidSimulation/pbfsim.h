#pragma once
#ifndef __pbfsim__
#define __pbfsim__
#include"predef.h"
#include"camera.h"
#include<glad/glad.h>

#include <vector>

class PBF 
{
private:
    
    const int _partical_count_;
    const float _box_size_;
    const float _partical_weight_;
    const float _fluid_density_;

    std::vector<glm::vec4> _partical_pos_;

public:
    Camera camera;

private:
    void predict();
    void solveConstraint();

public:
    PBF(int partical_count, float box_size, float partical_weight, float fluid_density);

    void initialize();
    void sim(double timestep);
    void render();

    
private:
    GLuint _buffer_partical_pos_prev_;
    GLuint _buffer_partical_pos_curr_;

    GLuint _partical_vao_;

// simulator kernels
private:
    GLuint _sim_predict_kernel_;

// renderer
private:
    GLuint _sphere_vertices_buffer_;
    GLuint _render_program_;

    GLuint _render_program_mView_location_;
    GLuint _render_program_mProjection_location_;

    void initializeRenderer();
    
};

#endif
