#pragma once

#ifndef __pbfsim__

#define __pbfsim__

#include"predef.h"
#include<glad/glad.h>

#include <vector>

// simulate the fluid inside a box of (-5~5,-5~5,-5~5)

class PBF 
{
private:
    
    const int _partical_count_;
    const float _partical_weight_;
    const float _fluid_density_;

    const float _kernel_size_;// diagnal of the kernel for density estimation
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
    void computeDensityConstraintPosDelta();
    void applyDensityConstraintPosDelta();

public:
    PBF(int partical_count, float partical_size, float fluid_density, float kernel_size);

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

public:
    GLuint getCurrPosVBO() { return _buffer_partical_pos_curr_; }


// simulator kernels
private:
    GLuint _sim_predict_kernel_;

    GLuint _sim_update_grid_kernel_;
    GLuint _sim_update_grid_cell_size_location_;
    GLuint _sim_update_grid_grid_edge_count_location_;
    GLuint _sim_update_grid_grid_edge_count2_location_;
    GLuint _sim_update_grid_cell_max_partical_count_location_;

    GLuint _sim_density_constraint_kernel_;
    GLuint _sim_density_constraint_cell_size_location_;
    GLuint _sim_density_constraint_grid_edge_count_location_;
    GLuint _sim_density_constraint_grid_edge_count2_location_;
    GLuint _sim_density_constraint_cell_max_partical_count_location_;

    GLuint _sim_apply_density_constraint_kernel_;

    // delta p
    GLuint _buffer_partical_pos_delta_;
    
};

#endif
