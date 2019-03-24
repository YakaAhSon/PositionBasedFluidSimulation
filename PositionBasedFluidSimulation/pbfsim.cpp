#include"pbfsim.h"
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

void PBF::sim(double timestep)
{
}

void PBF::render()
{
}
