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
    GLuint zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_partical_count_);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    glUseProgram(_sim_update_grid_kernel_);
    glUniform1f(_sim_update_grid_cell_size_location_, _grid_size_);
    glUniform1i(_sim_update_grid_grid_edge_count_location_, _grid_count_edge_);
    glUniform1i(_sim_update_grid_grid_edge_count2_location_, _grid_count_edge_*_grid_count_edge_);
    glUniform1i(_sim_update_grid_cell_max_partical_count_location_, static_cast<GLint>(_cell_max_partical_count_));
    

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);

    runComputeShaderForEachPartical();
}

void PBF::computeDensityConstraintPosDelta() 
{
    glUseProgram(_sim_density_constraint_kernel_);
    glUniform1f(_sim_density_constraint_cell_size_location_, _grid_size_);
    glUniform1i(_sim_density_constraint_grid_edge_count_location_, _grid_count_edge_);
    glUniform1i(_sim_density_constraint_grid_edge_count2_location_, _grid_count_edge_*_grid_count_edge_);
    glUniform1i(_sim_density_constraint_cell_max_partical_count_location_, _cell_max_partical_count_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer_partical_pos_delta_);

    runComputeShaderForEachPartical();
}

void PBF::applyDensityConstraintPosDelta()
{
    glUseProgram(_sim_apply_density_constraint_kernel_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_partical_pos_delta_);

    
    //float* delta = (float*)glMapNamedBuffer(_buffer_partical_pos_delta_, GL_READ_WRITE);
    //glUnmapNamedBuffer(_buffer_partical_pos_delta_);
    runComputeShaderForEachPartical();
}

PBF::PBF(int partical_count, float partical_size, float fluid_density, float kernel_size) :
    _partical_count_(partical_count),
    _partical_size_(partical_size),
    _partical_weight_(fluid_density*partical_size*partical_size*partical_size),
    _fluid_density_(fluid_density),
    _kernel_size_(kernel_size),
    _grid_size_(kernel_size * 0.7),
    _grid_count_edge_((int)(12 / (_grid_size_)) + 1),
    _grid_count_(_grid_count_edge_*_grid_count_edge_*_grid_count_edge_),
    _cell_max_partical_count_(pow(static_cast<int>(_kernel_size_ / _partical_size_) + 1, 3)*2)
    
{
    _partical_pos_.resize(_partical_count_);
}

void PBF::initialize()
{
    _partical_pos_.clear();
    for (float x = -5.0; x < 5.0; x+=_partical_size_) {
        for (float y = 0.0; y < 5.0; y += _partical_size_) {
            for (float z = -5.0; z < 5.0; z += _partical_size_){
                _partical_pos_.push_back(glm::vec4(x,y,z,0));
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
    _sim_update_grid_cell_size_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "cellsize");
    _sim_update_grid_grid_edge_count_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "grid_edge_count");
    _sim_update_grid_cell_max_partical_count_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "cellmaxparticalcount");
    _sim_update_grid_grid_edge_count2_location_ = glGetUniformLocation(_sim_update_grid_kernel_, "grid_edge_count2");
    
    _sim_density_constraint_kernel_ = util::createProgram_C(util::readFile("shaders\\density_constraint.glsl"));
    _sim_density_constraint_cell_size_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "cellsize");
    _sim_density_constraint_grid_edge_count_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "grid_edge_count");
    _sim_density_constraint_grid_edge_count2_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "grid_edge_count2");
    _sim_density_constraint_cell_max_partical_count_location_ = glGetUniformLocation(_sim_density_constraint_kernel_, "cellmaxparticalcount");

    _sim_apply_density_constraint_kernel_ = util::createProgram_C(util::readFile("shaders\\density_constraint_apply.glsl"));

    // cell buffers
    glGenBuffers(1, &_buffer_cell_partical_count_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_partical_count_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _grid_count_ * sizeof(GLuint), NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffer_cell_particals_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_particals_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint)*_grid_count_*_cell_max_partical_count_, NULL, GL_DYNAMIC_COPY);


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
