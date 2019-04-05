#pragma once

#ifndef __pbfsim__

#define __pbfsim__

#include"predef.h"
#include<glad/glad.h>

#include <vector>

#include"utilities.h"

// simulate the fluid inside a box of (-5~5,-5~5,-5~5)

class PBF 
{
private:
    
    const int _partical_count_;
    const float _partical_weight_;
    const float _fluid_density_;

    const float _kernel_radius_;// diagnal of the kernel for density estimation
    const float _partical_size_;// edge length of partical (assume particals are cubes), or diagnal of the partical spheres

    const float _grid_size_;// = _kernel_size_ * 0.7 #(_kernel_size_*0.5 should be sufficient)
    const int _grid_count_edge_;// ((int)(12/(_grid_size_))+1)
    const int _grid_count_; // _grid_count_edge_^3
    const int _cell_max_partical_count_; // (_kernel_size_ / _grid_size_ + 2)^3 * 2

    std::vector<glm::vec4> _partical_pos_;

    void runComputeShaderForEachPartical();
private:
    void predict();
    void updateGrid();
    void calculateLambda();
    void calculateDeltaP();
    void applyDensityConstraintPosDelta();

public:
    PBF(int partical_count, float partical_size, float fluid_density, float kernel_radius);

    int getParticalCount() { return _partical_count_; }

    void initialize();
    void sim(double timestep);

// buffers
private:
    GLuint _buffer_partical_pos_prev_;
    GLuint _buffer_partical_pos_curr_;

    // grid
    GLuint _buffer_cell_partical_count_;
    GLuint _buffer_cell_particals_;

    // partical index in array: _buffer_cell_particals_
    GLuint _buffer_partical_grid_index_;

    // delta p
    GLuint _buffer_partical_pos_delta_;

public:
    GLuint getCurrPosVBO() { return _buffer_partical_pos_curr_; }


// simulator kernels
private:
    class:public util::ComputeShader {
    public:
        virtual void initialize()override {
            createProgram("shaders\\predict.glsl");

        }
    }_sim_predict_kernel_;

    class:public util::ComputeShader {
    public:
        GLuint cell_size;
        GLuint grid_edge_count;
        GLuint grid_edge_count2;
        GLuint cell_max_partical_count;

        virtual void initialize()override {
            createProgram("shaders\\update_grid.glsl");
            cell_size = glGetUniformLocation(program, "cellsize");
            grid_edge_count = glGetUniformLocation(program, "grid_edge_count");
            cell_max_partical_count = glGetUniformLocation(program, "cellmaxparticalcount");
            grid_edge_count2 = glGetUniformLocation(program, "grid_edge_count2");
        }
    }_sim_update_grid_kernel_;

    class:public util::ComputeShader {
    public:
        virtual void initialize()override {
            createProgram("shaders\\density_constraint_apply.glsl");
        }
    }_sim_apply_density_constraint_kernel_;

    class:public util::ComputeShader {
    public:
        GLuint cellsize;
        GLuint grid_edge_count;
        GLuint cellmaxparticalcount;
        GLuint kernel_radius;

        virtual void initialize()override {
            createProgram("shaders\\calculate_lambda.glsl");
            cellsize = glGetUniformLocation(program, "cellsize");
            grid_edge_count = glGetUniformLocation(program, "grid_edge_count");
            cellmaxparticalcount = glGetUniformLocation(program, "cellmaxparticalcount");
            kernel_radius = glGetUniformLocation(program, "kernel_radius");
        }
    }_sim_cal_lambda_kernel_;

    class:public util::ComputeShader {
    public:

        GLuint cellsize;
        GLuint grid_edge_count;
        GLuint cellmaxparticalcount;
        GLuint kernel_radius;

        virtual void initialize()override {
            createProgram("shaders\\calculate_delta_p.glsl");

            cellsize = glGetUniformLocation(program, "cellsize");
            grid_edge_count = glGetUniformLocation(program, "grid_edge_count");
            cellmaxparticalcount = glGetUniformLocation(program, "cellmaxparticalcount");
            kernel_radius = glGetUniformLocation(program, "kernel_radius");
        }

    }_sim_cal_delta_p_kernel_;

    class :public util::ComputeShader {
    public:
        virtual void initialize()override {
            createProgram("shaders\\partical_to_grid.glsl");

        }
    }_sim_partical_to_grid_;
    void copyPosToGrid();
};

#endif
