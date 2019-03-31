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
    glUseProgram(_sim_predict_kernel_.program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_partical_pos_prev_);

    runComputeShaderForEachPartical();
}

void PBF::updateGrid()
{
    GLuint zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_partical_count_);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    glUseProgram(_sim_update_grid_kernel_.program);
    glUniform1f(_sim_update_grid_kernel_.cell_size, _grid_size_);
    glUniform1i(_sim_update_grid_kernel_.grid_edge_count, _grid_count_edge_);
    glUniform1i(_sim_update_grid_kernel_.grid_edge_count2, _grid_count_edge_*_grid_count_edge_);
    glUniform1i(_sim_update_grid_kernel_.cell_max_partical_count, static_cast<GLint>(_cell_max_partical_count_));
    

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);

    runComputeShaderForEachPartical();
}

void PBF::calculateLambda()
{
    _sim_cal_lambda_kernel_.use();
    glUniform1i(_sim_cal_lambda_kernel_.cellmaxparticalcount, _cell_max_partical_count_);
    glUniform1f(_sim_cal_lambda_kernel_.cellsize, _grid_size_);
    glUniform1f(_sim_cal_lambda_kernel_.kernel_radius, _kernel_radius_);
    glUniform1i(_sim_cal_lambda_kernel_.grid_edge_count, _grid_count_edge_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);

    runComputeShaderForEachPartical();
}

void PBF::calculateDeltaP()
{
    auto& kernel = _sim_cal_delta_p_kernel_;
    kernel.use();

    glUniform1f(kernel.cellsize, _grid_size_);
    glUniform1i(kernel.grid_edge_count, _grid_count_edge_);
    glUniform1i(kernel.cellmaxparticalcount, _cell_max_partical_count_);
    glUniform1f(kernel.kernel_radius, _kernel_radius_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer_partical_pos_delta_);

    runComputeShaderForEachPartical();
}

void PBF::applyDensityConstraintPosDelta()
{
    _sim_apply_density_constraint_kernel_.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_partical_pos_delta_);

    runComputeShaderForEachPartical();
}

PBF::PBF(int partical_count, float partical_size, float fluid_density, float kernel_radius) :
    _partical_count_(partical_count),
    _partical_size_(partical_size),
    _partical_weight_(fluid_density*partical_size*partical_size*partical_size),
    _fluid_density_(fluid_density),
    _kernel_radius_(kernel_radius),
    _grid_size_(kernel_radius *1.1),
    _grid_count_edge_((int)(12 / (_grid_size_)) + 1),
    _grid_count_(_grid_count_edge_*_grid_count_edge_*_grid_count_edge_),
    _cell_max_partical_count_(pow(static_cast<int>(kernel_radius*2 / _partical_size_) + 1, 3)*2)
    
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
    _sim_predict_kernel_.initialize();
    
    _sim_update_grid_kernel_.initialize();
    
    _sim_cal_lambda_kernel_.initialize();

    _sim_cal_delta_p_kernel_.initialize();

    _sim_apply_density_constraint_kernel_.initialize();

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
    for (int i = 0; i < 2; i++) {
        calculateLambda();
        calculateDeltaP();
        applyDensityConstraintPosDelta();
    }
}
