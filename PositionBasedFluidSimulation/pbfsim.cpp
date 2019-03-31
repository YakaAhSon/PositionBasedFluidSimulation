#include"pbfsim.h"

#include"utilities.h"

#include<iostream>



void PBF::runComputeShaderForEachPartical()
{
    glDispatchCompute(_partical_count_ / 128, 1, 1);
    glFinish();
}

void PBF::predict()
{
    glUseProgram(_sim_predict_kernel_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_partical_pos_prev_);

    runComputeShaderForEachPartical();
}

void PBF::updateGrid()
{
    GLuint zero = 0xffffffff;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_grid_partical_idx_);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    glUseProgram(_sim_update_grid_kernel_);
    glUniform1f(_sim_update_grid_grid_size_location_, _grid_size_);
    glUniform1i(_sim_update_grid_grid_edge_count_location_, _grid_count_edge_);
    glUniform1i(_sim_update_grid_grid_edge_count2_location_, _grid_count_edge_*_grid_count_edge_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_grid_partical_idx_);

    runComputeShaderForEachPartical();
}

void PBF::computeDensityConstraintPosDelta() 
{
    glUseProgram(_sim_density_constraint_kernel_);
    glUniform1f(_sim_density_constraint_grid_size_location_, _grid_size_);
    glUniform1i(_sim_density_constraint_grid_edge_count_location_, _grid_count_edge_);
    glUniform1i(_sim_density_constraint_grid_edge_count2_location_, _grid_count_edge_*_grid_count_edge_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_grid_partical_idx_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_partical_pos_delta_);

    runComputeShaderForEachPartical();
}

void PBF::applyDensityConstraintPosDelta()
{
    glUseProgram(_sim_apply_density_constraint_kernel_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_grid_partical_idx_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_partical_pos_delta_);

    
    //float* delta = (float*)glMapNamedBuffer(_buffer_partical_pos_delta_, GL_READ_WRITE);
    //glUnmapNamedBuffer(_buffer_partical_pos_delta_);
    runComputeShaderForEachPartical();
}

PBF::PBF(int partical_count, float partical_size, float fluid_density):
    _partical_count_(partical_count),
    _partical_size_(partical_size),
    _partical_weight_(fluid_density*partical_size*partical_size*partical_size),
    _fluid_density_(fluid_density),
    _grid_size_(partical_size /1.5),
    _grid_count_edge_((int)(10 / (_grid_size_)) + 1),
    _grid_count_(_grid_count_edge_*_grid_count_edge_*_grid_count_edge_)
{
    _partical_pos_.resize(_partical_count_);
}

void PBF::initialize()
{
    _partical_pos_.clear();
    for (int x = -25; x < 25; x++) {
        for (int y = 0; y < 25; y++) {
            for (int z = -25; z < 25; z++) {
                _partical_pos_.push_back(glm::vec4(x/5.0, y/5.0, z/5.0,0));
            }
        }
    }


    glGenBuffers(1, &_buffer_partical_pos_prev_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_prev_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, &_partical_pos_[0], GL_DYNAMIC_READ);

    glGenBuffers(1, &_buffer_partical_pos_curr_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_curr_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, &_partical_pos_[0], GL_DYNAMIC_READ);

    glGenBuffers(1, &_buffer_partical_pos_delta_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_delta_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, NULL, GL_DYNAMIC_READ);

    // initialize simulator kernels
    _sim_predict_kernel_ = util::createProgram_C(util::readFile("shaders\\predict.glsl"));
    
    _sim_update_grid_kernel_ = util::createProgram_C(util::readFile("shaders\\update_grid.glsl"));
    _sim_update_grid_grid_size_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "grid_size");
    _sim_update_grid_grid_edge_count_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "grid_edge_count");
    _sim_update_grid_grid_edge_count2_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "grid_edge_count2");
    
    _sim_density_constraint_kernel_ = util::createProgram_C(util::readFile("shaders\\density_constraint.glsl"));
    _sim_density_constraint_grid_size_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "grid_size");
    _sim_density_constraint_grid_edge_count_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "grid_edge_count");
    _sim_density_constraint_grid_edge_count2_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "grid_edge_count2");

    _sim_apply_density_constraint_kernel_ = util::createProgram_C(util::readFile("shaders\\density_constraint_apply.glsl"));

    glGenBuffers(1, &_buffer_grid_partical_idx_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_grid_partical_idx_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _grid_count_ * sizeof(GLuint), NULL, GL_DYNAMIC_COPY);
}

void PBF::sim(double timestep)
{
    predict();

    updateGrid();
    for (int i = 0; i < 3; i++) {
        computeDensityConstraintPosDelta();
        applyDensityConstraintPosDelta();
    }
}
