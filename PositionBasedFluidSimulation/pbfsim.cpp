#include"pbfsim.h"

#include"utilities.h"

#include<vector>
#include<iostream>

using namespace util;


void PBF::predict()
{
    static GLuint program = createProgram_C(readFile("shaders\\predict.glsl"));


    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_partical_pos_prev_);

    runForAllParticals();
}

void PBF::updateGrid()
{
    static GLuint program = util::createProgram_C(readFile("shaders\\update_grid.glsl"));
    bindUniformLocation(cellsize);
    bindUniformLocation(grid_edge_count);
    bindUniformLocation(cellmaxparticalcount);
    bindUniformLocation(grid_edge_count2);

    GLuint zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_partical_count_);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    glUseProgram(program);
    glUniform1f(cellsize, _grid_size_);
    glUniform1i(grid_edge_count, _grid_count_edge_);
    glUniform1i(grid_edge_count2, _grid_count_edge_*_grid_count_edge_);
    glUniform1i(cellmaxparticalcount, static_cast<GLint>(_cell_max_partical_count_));
    

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer_partical_grid_index_);

    runForAllParticals();
}

void PBF::applyBoundaryConstraint()
{
    static GLuint program = createProgram_C(readFile("shaders\\boundary_constraint.glsl"));
    bindUniformLocation(left_boundary);


    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);

    static float timer = 0;

    float left = -7.0 - glm::cos(timer);

    glUniform1f(left_boundary, left);
    timer += 1 / 20.0;
    runForAllParticals();
}

void PBF::calculateLambda()
{
    static GLuint program  = util::createProgram_C(readFile("shaders\\calculate_lambda.glsl"));
    bindUniformLocation(cellsize);
    bindUniformLocation(grid_edge_count);
    bindUniformLocation(cellmaxparticalcount);
    bindUniformLocation(kernel_radius);

    glUseProgram(program);
    glUniform1i(cellmaxparticalcount, _cell_max_partical_count_);
    glUniform1f(cellsize, _grid_size_);
    glUniform1f(kernel_radius, _kernel_radius_);
    glUniform1i(grid_edge_count, _grid_count_edge_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer_partical_grid_index_);

    runForAllParticals();
}

void PBF::calculateDeltaP()
{
    static GLuint program = util::createProgram_C(readFile("shaders\\calculate_delta_p.glsl"));

    bindUniformLocation(cellsize);
    bindUniformLocation(grid_edge_count);
    bindUniformLocation(cellmaxparticalcount);
    bindUniformLocation(kernel_radius);

    glUseProgram(program);

    glUniform1f(cellsize, _grid_size_);
    glUniform1i(grid_edge_count, _grid_count_edge_);
    glUniform1i(cellmaxparticalcount, _cell_max_partical_count_);
    glUniform1f(kernel_radius, _kernel_radius_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer_partical_pos_delta_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _buffer_partical_grid_index_);

    runForAllParticals();
}

void PBF::applyDensityConstraintPosDelta()
{
    static GLuint program = util::createProgram_C(readFile("shaders\\density_constraint_apply.glsl"));

    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_partical_pos_delta_);

    runForAllParticals();
}

void PBF::copyPosToGrid()
{
    static GLuint program = util::createProgram_C(readFile("shaders\\partical_to_grid.glsl"));

    glUseProgram(program);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_partical_pos_curr_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_cell_partical_count_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_cell_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer_partical_grid_index_);

    runForAllParticals();
}

PBF::PBF(int partical_count, float partical_size, float fluid_density, float kernel_radius) :
    _partical_count_(partical_count),
    _partical_size_(partical_size),
    _partical_weight_(fluid_density*partical_size*partical_size*partical_size),
    _fluid_density_(fluid_density),
    _kernel_radius_(kernel_radius),
    _grid_size_(kernel_radius),
    _grid_count_edge_((int)(12 / (_grid_size_)) + 1),
    _grid_count_(_grid_count_edge_*_grid_count_edge_*_grid_count_edge_*2),
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
    _partical_pos_.resize(_partical_count_);

    


    glGenBuffers(1, &_buffer_partical_pos_prev_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_prev_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, &_partical_pos_[0], GL_DYNAMIC_READ);

    glGenBuffers(1, &_buffer_partical_pos_curr_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_curr_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, &_partical_pos_[0], GL_DYNAMIC_READ);

    glGenBuffers(1, &_buffer_partical_pos_delta_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_pos_delta_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4)*_partical_count_, NULL, GL_DYNAMIC_READ);

       
    // cell buffers
    glGenBuffers(1, &_buffer_cell_partical_count_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_partical_count_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _grid_count_ * sizeof(GLuint), NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffer_cell_particals_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_cell_particals_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (sizeof(GLuint)+sizeof(glm::vec3))*_grid_count_*_cell_max_partical_count_, NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffer_partical_grid_index_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_partical_grid_index_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint)*_partical_count_, NULL, GL_DYNAMIC_COPY);

    aball.loadModel("assets\\bunny.obj");
    aball.setMass(100.0);
    aball.voxelize();
}

void PBF::sim(double timestep)
{

    aball.predict();

    constexpr int steps_per_frame = 5;

//#define SHOW_KERNEL_TIMES

#if defined SHOW_KERNEL_TIMES
    util::Timer t;
    predict();
    t.toc("Predict");
    t.tic();
    updateGrid();
    t.toc("Update Grid");

    applyBoundaryConstraint();

    for (int i = 0; i < steps_per_frame; i++) {
        t.tic();
        copyPosToGrid();
        t.toc("Copy Pos");
        t.tic();
        calculateLambda();
        t.toc("Cal Lambda");
        t.tic();
        calculateDeltaP();
        t.toc("Cal Pos Delta");
        t.tic();
        applyDensityConstraintPosDelta();
        t.toc("Apply Delta");
    }
    std::cout << std::endl;
#else
    predict();
    updateGrid();
    applyBoundaryConstraint();
    aball.runConstraint(_buffer_partical_pos_curr_,_partical_count_);
    for (int i = 0; i < steps_per_frame; i++) {
        copyPosToGrid();
        calculateLambda();
        calculateDeltaP();
        applyDensityConstraintPosDelta();

        aball.runConstraint(_buffer_partical_pos_curr_, _partical_count_);
    }
#endif

}

void PBF::runForAllParticals()
{
    glDispatchCompute(_partical_count_ / 128, 1, 1);
    glFinish();
}


