#pragma once

#ifndef __pbfsim__

#define __pbfsim__

#include"predef.h"
#include<glad/glad.h>

#include <vector>

#include"utilities.h"

#include"solidmodel.h"

// simulate the fluid inside a box of (-5~5,-5~5,-5~5)

class PBF 
{
private:
    
    // solid particals
    // not constant, because it depends on the scene
    int _solid_partical_count_;

    // fluid particals
    const int _partical_count_;
    const float _partical_weight_;
    const float _fluid_density_;

    const float _kernel_radius_;// diagnal of the kernel for density estimation
    const float _partical_size_;// edge length of partical (assume particals are cubes), or diagnal of the partical spheres

    const float _grid_size_;// = _kernel_size_ * 0.7 #(_kernel_size_*0.5 should be sufficient)
    const int _grid_count_edge_;// ((int)(12/(_grid_size_))+1)
    const int _grid_count_; // _grid_count_edge_^3*2

private:

    void runForAllParticals();

    void predict();
    void updateGrid();
    void calculateLambda();
    void calculateDeltaP();
    void applyDensityConstraintPosDelta();
    void applyBoundaryConstraint();

public:
    PBF(int partical_count, float partical_size, float fluid_density, float kernel_radius);

    int getParticalCount() { return _partical_count_; }

    void initialize();
    void sim(double timestep);

// buffers
private:
    // Partical Data Structure only in GPU memory
    using Partical = struct {
        glm::vec3 pos;
        float lambda;
        glm::vec3 pos_prev;

        unsigned int grid_idx;

        glm::vec3 delta_p;
        unsigned int idx_in_grid;
    };
    GLuint _buffer_particals_;
    GLuint _buffer_particals_tmp_;

    // unsigned int grid[] 
    // used as both count and start
    // count is calculated by GPU
    // then download to cpu and calculate start iteratively
    // then upload the list of grid start to GPU, and used to "sort" partical buffer
    using Grid = struct {
        GLuint partical_count;
        GLuint partical_start;
    };
    GLuint _buffer_grids_;

public:
    const GLuint getCurrPosVBO()const { return _buffer_particals_; }


public:
    std::vector<SolidModel*> _solid_models_;
};

#endif
