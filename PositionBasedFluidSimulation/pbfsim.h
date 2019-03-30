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

    const float _partical_size_;// edge length of partical (assume particals are cubes)

    const float _grid_size_;// = _partical_size_
    const int _grid_count_edge_;// ((int)(10/(_grid_size_))+1)
    const int _grid_count_; // _grid_count_edge_^3

    std::vector<glm::vec4> _partical_pos_;


private:
    void predict();
    void updateGrid();
    void solveConstraint();

public:
    PBF(int partical_count, float partical_radius, float fluid_density);

    int getParticalCount() { return _partical_count_; }

    void initialize();
    void sim(double timestep);

// buffers
private:
    GLuint _buffer_partical_pos_prev_;
    GLuint _buffer_partical_pos_curr_;

    // cell size = partical radius
    GLuint _buffer_grid_partical_idx_;
public:
    GLuint getCurrPosVBO() { return _buffer_partical_pos_curr_; }


// simulator kernels
private:
    GLuint _sim_predict_kernel_;
    GLuint _sim_border_constraint_kernel_;

    GLuint _sim_update_grid_kernel_;
    GLuint _sim_update_grid_grid_size_location_;
    GLuint _sim_update_grid_grid_edge_count_location_;
    GLuint _sim_update_grid_grid_edge_count2_location_;

    GLuint _sim_density_constraint_kernel_;
    GLuint _sim_density_constraint_grid_size_location_;
    GLuint _sim_density_constraint_grid_edge_count_location_;
    GLuint _sim_density_constraint_grid_edge_count2_location_;
    
};

#endif
