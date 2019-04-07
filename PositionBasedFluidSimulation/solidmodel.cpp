#include "solidmodel.h"

#include"utilities.h"

void SolidModel::voxelize()
{
    static GLuint program = []() {
        GLuint program = util::createProgram_VF(
            util::readFile("shaders\\solid_genvoxel_vertex.glsl"),
            util::readFile("shaders\\solid_genvoxel_fragment.glsl")
        );
        glBindAttribLocation(program, 0, "vVertex");
        glBindAttribLocation(program, 1, "vNormal");
        util::linkProgram(program);
        return program;
    }(); 

    bindUniformLocation(erodeDistance);
    bindUniformLocation(voxelSize);
    bindUniformLocation(bBoxMin);
    bindUniformLocation(voxelSpaceSize);

    glGenBuffers(1, &_voxel_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _voxel_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _voxel_space_size_.x*_voxel_space_size_.y*_voxel_space_size_.z* sizeof(Voxel), NULL, GL_STATIC_DRAW);
    GLuint zero = 0;
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _voxel_buffer_);

    glViewport(0, 0, 128, 128);

    glUseProgram(program);
    glBindVertexArray(_vao_);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glUniform1f(voxelSize, _voxel_size_);
    glUniform3fv(bBoxMin, 1, &_bmin_[0]);
    glUniform3iv(voxelSpaceSize, 1, &_voxel_space_size_[0]);

    glUniform1f(erodeDistance, _voxel_size_*0.5);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());

    glUniform1f(erodeDistance, _voxel_size_*1.3);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());


    glBindVertexArray(0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

GLuint SolidModel::_impulse_buffer_ = 0;
GLuint SolidModel::_impulse_counter_buffer_ = 0;
void SolidModel::runConstraint(GLuint partical_pos_buffer, int partical_count)
{
    updateModelViewMatrices();
    static GLuint program = util::createProgram_C(util::readFile("shaders\\solid_constraint.glsl"));
    bindUniformLocation(bBoxMin);
    bindUniformLocation(voxelSpaceSize);
    bindUniformLocation(voxelSize);
    bindUniformLocation(mView);
    bindUniformLocation(mModelRot);

    static GLuint impulse_buffer = [&]() {
        glGenBuffers(1, &_impulse_buffer_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _impulse_buffer_);
        
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FluidImpulse)*_max_impulses_,NULL,GL_STATIC_READ);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return _impulse_buffer_;
    }();

    static GLuint impulse_counter_buffer = [&]() {
        glGenBuffers(1, &_impulse_counter_buffer_);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _impulse_counter_buffer_);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_READ);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        return _impulse_counter_buffer_;
    }();

    GLuint zero = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _impulse_counter_buffer_);
    glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _impulse_counter_buffer_);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partical_pos_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _voxel_buffer_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _impulse_buffer_);

    glUseProgram(program);
    glUniform3fv(bBoxMin, 1, &_bmin_[0]);
    glUniform3iv(voxelSpaceSize, 1, &_voxel_space_size_[0]);
    glUniform1f(voxelSize, _voxel_size_);
    glUniformMatrix4fv(mView, 1, GL_FALSE, &_mView_[0][0]);
    glUniformMatrix3fv(mModelRot, 1, GL_FALSE, &_mModelRot_[0][0]);

    glDispatchCompute(partical_count / 128, 1, 1);
    glFinish();

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _impulse_counter_buffer_);
    GLuint* pImpulseCounter = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
    GLuint impulse_number = pImpulseCounter[0];
    pImpulseCounter[0] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    //std::cout << "impulse number: " << impulse_number << std::endl;
   

    static std::vector<FluidImpulse> impulses(_max_impulses_);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _impulse_buffer_);
    
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, impulse_number * sizeof(FluidImpulse), &impulses[0]);

    //util::Timer t;
    int step = impulse_number / 500;
    step = glm::max(step, 1);
    for (int i = 0; i < impulse_number; i+=step) {
        positionImpulse(impulses[i].pos, impulses[i].normal, impulses[i].depth*0.01);
    }
    //t.toc("interaction");
}

void SolidModel::render(Camera& camera)
{

    updateModelViewMatrices();

    static GLuint program = []() {
        GLuint program = util::createProgram_VF(util::readFile("shaders\\mesh_render_vertex.glsl"),
            util::readFile("shaders\\mesh_render_fragment.glsl"));
        glBindAttribLocation(program, 0, "vVertex");
        glBindAttribLocation(program, 1, "vNorm");
        glLinkProgram(program);
        return program;
    }();

    static GLuint mVPLocation = glGetUniformLocation(program, "mMVP");

    glUseProgram(program);
    glm::mat4 mMVP = camera.getViewProjectionMatrix()*getMModel();

    glUniformMatrix4fv(mVPLocation, 1,GL_FALSE, &mMVP[0][0]);

    glBindVertexArray(_vao_);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());
    glBindVertexArray(0);
}
