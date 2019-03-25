#include"pbfsim.h"
#include"utilities.h"

void PBF::predict()
{
}

void PBF::solveConstraint() 
{
}

PBF::PBF(int partical_count, float box_size, float partical_weight, float fluid_density):
    _partical_count_(partical_count),
    _box_size_(box_size),
    _partical_weight_(partical_weight),
    _fluid_density_(fluid_density)
{
    _partical_pos_.resize(_partical_count_);
    _partical_pos_prev_.resize(_partical_count_);
}

void PBF::initialize()
{
    _partical_pos_.clear();
    for (int x = -50; x < 50; x++) {
        for (int y = -50; y < 0; y++) {
            for (int z = -50; z < 50; z++) {
                _partical_pos_.push_back(glm::vec3(x/10.0, y/10.0, z/10.0));
            }
        }
    }
    initializeRenderer();
}

void PBF::sim(double timestep)
{
    predict();
    solveConstraint();
}


