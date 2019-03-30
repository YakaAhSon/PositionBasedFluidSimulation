#include"pbfsim.h"
#include"utilities.h"

void PBF::predict()
{
    glUseProgram(_sim_predict_kernel_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_prev_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_partical_pos_curr_);

    glDispatchCompute(_partical_count_ / 32, 1, 1);
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
}

void PBF::initialize()
{
    _partical_pos_.clear();
    for (int x = -50; x < 50; x++) {
        for (int y = -50; y < 0; y++) {
            for (int z = -50; z < 50; z++) {
                _partical_pos_.push_back(glm::vec4(x/10.0, y/10.0+5.0, z/10.0,0));
            }
        }
    }


    glGenBuffers(1, &_buffer_partical_pos_prev_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_prev_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, &_partical_pos_[0], GL_DYNAMIC_DRAW);

    glGenBuffers(1, &_buffer_partical_pos_curr_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_curr_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, &_partical_pos_[0], GL_DYNAMIC_DRAW);

    // initialize simulato rkernels
    _sim_predict_kernel_ = util::createProgram_C(util::readFile("shaders\\predict.glsl"));


    initializeRenderer();
    
}

void PBF::sim(double timestep)
{
    predict();
    solveConstraint();
}


