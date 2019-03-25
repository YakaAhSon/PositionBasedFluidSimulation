#include"pbfsim.h"
#include"utilities.h"

void PBF::initializeRenderer()
{
    _render_program_ = util::createProgram_VF(
        util::readFile("shaders\\fluid_render_vertex.glsl"),
        util::readFile("shaders\\fluid_render_fragment.glsl"));

    glBindAttribLocation(_render_program_, 0, "vVertex");
    util::linkProgram(_render_program_);
    _render_program_mMVP_location_ = glGetUniformLocation(_render_program_, "mVP");

    glGenVertexArrays(1, &_partical_vao_);
    glBindVertexArray(_partical_vao_);

    glGenBuffers(1, &_partical_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, _partical_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*_partical_count_, NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
}

void PBF::render()
{
    glUseProgram(_render_program_);

    glUniformMatrix4fv(_render_program_mMVP_location_, 1, GL_FALSE, &camera.getViewProjectionMatrix()[0][0]);
    glBindVertexArray(_partical_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, _partical_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3)*_partical_count_, &_partical_pos_[0]);

    glPointSize(1);
    glDrawArrays(GL_POINTS, 0, _partical_count_);

    glBindVertexArray(0);
}