#include"pbfsim.h"

#include"utilities.h"

#include<vector>
#include<iostream>

#include"boundary.h"

using namespace util;


void PBF::predict()
{
    static GLuint program = createProgram_C(readFile("shaders\\predict.glsl"));


    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);

    runForAllParticals();
}

void PBF::updateGrid()
{
    /* schema:
    step1. GPU: find the grid index of each partical, count partical number of each grid
    step2. Download grid_count_buffer to local memory
    step3. CPU: calculate grid start for each grid
    step4. Upload grid_start_buffer to GOU memory
    step5. GPU: "Sort" particals according to grid. Save to _tmp_
    step6. swap partical_buffer and partical_buffer_tmp
    */

    // step1. GPU: find grid index, count particals for each grid
    static GLuint program = util::createProgram_C(readFile("shaders\\update_grid_findgrid.glsl"));
    bindUniformLocation(cellsize);
    bindUniformLocation(grid_edge_count);
    bindUniformLocation(grid_edge_count2);

    GLuint zero = 0;
    glClearNamedBufferData(_buffer_grids_, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    glUseProgram(program);
    glUniform1f(cellsize, _grid_size_);
    glUniform1i(grid_edge_count, _grid_count_edge_);
    glUniform1i(grid_edge_count2, _grid_count_edge_*_grid_count_edge_);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_grids_);
    runForAllParticals();

    // step2. Download grid_count_buffer to local memory
    static std::vector<Grid> grids(_grid_count_);
    glGetNamedBufferSubData(_buffer_grids_, 0, _grid_count_ * sizeof(Grid), &grids[0]);

    // step3. CPU: calculate grid start for each grid
    GLuint start = 0;
    for (Grid& g : grids) {
        g.partical_start = start;
        start += g.partical_count;
    }

    // step4. upload grids
    glNamedBufferSubData(_buffer_grids_, 0, sizeof(Grid)*_grid_count_, &grids[0]);

    // step5. "Sort" particals
    static GLuint program_sort = util::createProgram_C(readFile("shaders\\update_grid_sort.glsl"));

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_particals_tmp_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer_grids_);

    glUseProgram(program_sort);
    runForAllParticals();

    // swap
    std::swap(_buffer_particals_, _buffer_particals_tmp_);
}

void PBF::applyBoundaryConstraint()
{
    static GLuint program = createProgram_C(readFile("shaders\\boundary_constraint.glsl"));
    bindUniformLocation(bmin);
    bindUniformLocation(bmax);


    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);

    glUniform3fv(bmin,1, &boundary::bmin[0]);
    glUniform3fv(bmax,1, &boundary::bmax[0]);
    runForAllParticals();
}

void PBF::calculateLambda()
{
    static GLuint program  = util::createProgram_C(readFile("shaders\\calculate_lambda.glsl"));
    bindUniformLocation(cellsize);
    bindUniformLocation(grid_edge_count);
    bindUniformLocation(kernel_radius);

    glUseProgram(program);
    glUniform1f(cellsize, _grid_size_);
    glUniform1f(kernel_radius, _kernel_radius_);
    glUniform1i(grid_edge_count, _grid_count_edge_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_grids_);

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
    glUniform1f(kernel_radius, _kernel_radius_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer_grids_);

    runForAllParticals();
}

void PBF::applyDensityConstraintPosDelta()
{
    static GLuint program = util::createProgram_C(readFile("shaders\\density_constraint_apply.glsl"));

    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffer_particals_);

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
    _grid_count_(_grid_count_edge_*_grid_count_edge_*_grid_count_edge_*2)    
{
}

void PBF::initialize()
{
    std::vector<Partical> particals;
    particals.reserve(_partical_count_);
    for (float y = 5.0; y > -5.0; y-=_partical_size_) {
        for (float x = -7.0; x < 7.0; x += _partical_size_) {
            for (float z = -5.0; z < 5.0; z += _partical_size_){
                particals.push_back({
                    glm::vec3(x,y,z) ,
                    0,
                    glm::vec3(x,y,z),
                    });
            }
        }
    }
    particals.resize(_partical_count_);


    glGenBuffers(1, &_buffer_particals_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_particals_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Partical)*_partical_count_, &particals[0], GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffer_particals_tmp_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_particals_tmp_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Partical)*_partical_count_, NULL, GL_DYNAMIC_COPY);

    // cell buffers
    glGenBuffers(1, &_buffer_grids_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer_grids_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _grid_count_ * sizeof(Grid), NULL, GL_STREAM_READ);


    SolidModel* cylinder = SolidModel::loadModel("assets\\cylinder.obj", 1, 20000.0);
    cylinder->moveGlobal(glm::vec3(9, 0, -4));

    cylinder = SolidModel::loadModel("assets\\cylinder.obj", 1, 20000.0);
    cylinder->moveGlobal(glm::vec3(9, 0, 4));

    cylinder = SolidModel::loadModel("assets\\cylinder_h.obj", 1, 20000.0);
    cylinder->moveGlobal(glm::vec3(9, 4, 0));

    cloth = new Cloth(10, 10, glm::vec3(9, 4, -4), glm::vec3(9, 4, 4), glm::vec3(9, -4, -4));

}

void PBF::sim(double timestep)
{

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

    SolidModel::predictAll(cloth);
    predict();
    applyBoundaryConstraint();

    updateGrid();

    cloth->predict();

    for (int i = 0; i < steps_per_frame; i++) {
        calculateLambda();
        calculateDeltaP();
        applyDensityConstraintPosDelta();

        SolidModel::runConstraintsAll(_buffer_particals_, _partical_count_);
    }
    cloth->blowByFluid(_buffer_particals_, _partical_count_);
#endif

}

SolidModel* PBF::addObject(const char * modelFile, float mass)
{
    auto* d = SolidModel::loadModel(modelFile,0,mass);
    d->moveGlobal(glm::vec3(0, 1, 0));
    return d;
}

void PBF::runForAllParticals()
{
    glDispatchCompute(_partical_count_ / 128, 1, 1);
    glFinish();
}


