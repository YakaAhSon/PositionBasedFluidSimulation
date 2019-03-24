#pragma once
#ifndef __pbfsim__
#define __pbfsim__

#include "glm/glm.hpp"

#include <vector>

class PBF 
{
private:
    
    const int _partical_count_;
    const float _box_size_;
    const float _partical_weight_;
    const float _fluid_density_;

    std::vector<glm::vec3> _partical_pos_;
    std::vector<glm::vec3> _partical_pos_prev_;

private:
    void predict();
    void solveConstraint();

public:
    PBF(int partical_count, float box_size, float partical_weight, float fluid_density);
    void sim(double timestep);
    void render();
};

#endif
