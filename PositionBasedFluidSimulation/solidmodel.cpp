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

    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glViewport(0, 0, 512, 512);

    glUseProgram(program);
    glBindVertexArray(_vao_);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glUniform1f(voxelSize, _voxel_size_);
    glUniform3fv(bBoxMin, 1, &_bmin_[0]);
    glUniform3iv(voxelSpaceSize, 1, &_voxel_space_size_[0]);

    glUniform1f(erodeDistance, _voxel_size_*0.1);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());

    glUniform1f(erodeDistance, _voxel_size_*0.6);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());


    glUniform1f(erodeDistance, _voxel_size_*1.1);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());

    glUniform1f(erodeDistance, _voxel_size_*1.6);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _voxel_buffer_);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, _voxels_.size() * sizeof(Voxel), &_voxels_[0]);

    glBindVertexArray(0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glViewport(vp[0], vp[1], vp[2], vp[3]);
}

GLuint SolidModel::_impulse_buffer_ = 0;
GLuint SolidModel::_impulse_counter_buffer_ = 0;


glm::vec3 SolidModel::getGlobalPos(glm::vec3 pos)
{
    return _COM_ + glm::rotate(_orientation_, pos);
}


glm::vec3 SolidModel::getLocalPos(glm::vec3 pos)
{
    glm::vec3 delta = pos - _COM_;

    glm::quat invO = glm::inverse(_orientation_);

    return glm::rotate(invO, delta);
}

void SolidModel::runConstraintsAll(GLuint partical_buffer, int partical_count)
{
    static GLuint program = util::createProgram_C(util::readFile("shaders\\solid_constraint.glsl"));
    bindUniformLocation(bBoxMin);
    bindUniformLocation(voxelSpaceSize);
    bindUniformLocation(voxelSize);
    bindUniformLocation(mView);
    bindUniformLocation(mModelRot);
    bindUniformLocation(isFixed);

    static GLuint impulse_buffer = [&]() {
        glGenBuffers(1, &_impulse_buffer_);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _impulse_buffer_);
        
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FluidImpulse)*_max_impulses_,NULL,GL_DYNAMIC_READ);

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

    // common settings for fixed and unfixed modles
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partical_buffer);
    glUseProgram(program);
    glUniform1f(voxelSize, _voxel_size_);

    // run constraint for fixed models
    glUniform1i(isFixed, 1);
    for (SolidModel* m : _fixed_models_) {
        m->updateModelViewMatrices();
        glUniform3fv(bBoxMin, 1, &m->_bmin_[0]);
        glUniform3iv(voxelSpaceSize, 1, &m->_voxel_space_size_[0]);
        glUniformMatrix4fv(mView, 1, GL_FALSE, &m->_mView_[0][0]);
        glUniformMatrix3fv(mModelRot, 1, GL_FALSE, &m->_mModelRot_[0][0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m->_voxel_buffer_);
        glDispatchCompute(partical_count / 128, 1, 1);
        glFinish();
    }

    // run constraints for unfixed models and fetch fluid impulses
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _impulse_counter_buffer_);
    GLuint* pImpulseCounter = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
    pImpulseCounter[0] = 0;
    GLuint impulseCounter = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _impulse_counter_buffer_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _impulse_buffer_);

    glUniform1i(isFixed, 0);
    for (SolidModel*m : _unfixed_models_) {
        m->updateModelViewMatrices();
        glUniform3fv(bBoxMin, 1, &m->_bmin_[0]);
        glUniform3iv(voxelSpaceSize, 1, &m->_voxel_space_size_[0]);
        glUniformMatrix4fv(mView, 1, GL_FALSE, &m->_mView_[0][0]);
        glUniformMatrix3fv(mModelRot, 1, GL_FALSE, &m->_mModelRot_[0][0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m->_voxel_buffer_);
        glDispatchCompute(partical_count / 128, 1, 1);
        glFinish();

        m->_impulse_offset_ = impulseCounter;

        pImpulseCounter = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
        impulseCounter = pImpulseCounter[0];
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

        m->_impulse_count_ = impulseCounter - m->_impulse_offset_;
    }

    // apply fluid impulse for unfixed models
    // fetch all impulses once, to reduce GPU memory latency
    static std::vector<FluidImpulse> impulses(_max_impulses_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _impulse_buffer_);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, impulseCounter * sizeof(FluidImpulse), &impulses[0]);
    // apply impulse for each model
    for (SolidModel* m : _unfixed_models_) {
        int step = m->_impulse_count_ / 500;// sample some impulses
        step = glm::max(step, 1);
        for (int i = 0; i < m->_impulse_count_; i += step) {
            const FluidImpulse& impulse = impulses[i + m->_impulse_offset_];
            m->positionImpulse(impulse.pos, impulse.normal, impulse.depth*2.0 * step/m->getMass());
        }
    }
    
}

const std::vector<SolidModel*>& SolidModel::getModels()
{
    return _unfixed_models_;
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
    bindUniformLocation(mModelRot);
    bindUniformLocation(color);

    glUseProgram(program);
    glm::mat4 mMVP = camera.getViewProjectionMatrix()*getMModel();

    glUniformMatrix4fv(mVPLocation, 1,GL_FALSE, &mMVP[0][0]);

    glUniformMatrix3fv(mModelRot, 1, GL_FALSE, &_mModelRot_[0][0]);

    float colors[3] = { 1.0,0.7,0.4 };
    glUniform3fv(color, 1, colors);


    glBindVertexArray(_vao_);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());
    glBindVertexArray(0);
}
