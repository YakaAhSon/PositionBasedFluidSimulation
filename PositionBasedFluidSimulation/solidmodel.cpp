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

void SolidModel::runConstraint(GLuint partical_pos_buffer, int partical_count)
{
    static GLuint program = util::createProgram_C(util::readFile("shaders\\solid_constraint.glsl"));
    bindUniformLocation(bBoxMin);
    bindUniformLocation(voxelSpaceSize);
    bindUniformLocation(voxelSize);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partical_pos_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _voxel_buffer_);

    glUseProgram(program);
    glUniform3fv(bBoxMin, 1, &_bmin_[0]);
    glUniform3iv(voxelSpaceSize, 1, &_voxel_space_size_[0]);
    glUniform1f(voxelSize, _voxel_size_);

    glDispatchCompute(partical_count / 128, 1, 1);
}

void SolidModel::render(Camera& camera)
{
    static GLuint program = []() {
        GLuint program = util::createProgram_VF(util::readFile("shaders\\mesh_render_vertex.glsl"),
            util::readFile("shaders\\mesh_render_fragment.glsl"));
        glBindAttribLocation(program, 0, "vVertex");
        glBindAttribLocation(program, 1, "vNorm");
        glLinkProgram(program);
        return program;
    }();

    static GLuint mVPLocation = glGetUniformLocation(program, "mViewProjection");

    glUseProgram(program);
    glm::mat4 mVP = camera.getViewProjectionMatrix();

    glUniformMatrix4fv(mVPLocation, 1,GL_FALSE, &mVP[0][0]);

    glBindVertexArray(_vao_);
    glDrawArrays(GL_TRIANGLES, 0, _mesh_.size());
    glBindVertexArray(0);
}
