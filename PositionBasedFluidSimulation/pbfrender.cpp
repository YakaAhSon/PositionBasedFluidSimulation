#include"pbfsim.h"
#include"utilities.h"

static const int sphere_details = 8;

void PBF::initializeRenderer()
{
    _render_program_ = util::createProgram_VF(
        util::readFile("shaders\\fluid_render_vertex.glsl"),
        util::readFile("shaders\\fluid_render_fragment.glsl"));

    glBindAttribLocation(_render_program_, 0, "vVertex");
    glBindAttribLocation(_render_program_, 1, "particalPosition");
    util::linkProgram(_render_program_);
    _render_program_mView_location_ = glGetUniformLocation(_render_program_, "mView");
    _render_program_mProjection_location_ = glGetUniformLocation(_render_program_, "mProjection");

    glGenVertexArrays(1, &_partical_vao_);
    glBindVertexArray(_partical_vao_);

    glGenBuffers(1, &_sphere_vertices_buffer_);
    std::vector<glm::vec2> sphere_vertices;

    sphere_vertices.push_back(glm::vec2(0, 0));
    for (int i = 0; i < sphere_details+1; i++) {
        float a1 = 2*glm::pi<float>() / sphere_details *i;
        sphere_vertices.push_back(glm::vec2(glm::cos(a1), glm::sin(a1)));
    }
    glBindBuffer(GL_ARRAY_BUFFER, _sphere_vertices_buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*sphere_vertices.size(), &sphere_vertices[0][0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, _buffer_partical_pos_curr_);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

void PBF::render()
{
    glUseProgram(_render_program_);

    glUniformMatrix4fv(_render_program_mView_location_, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
    glUniformMatrix4fv(_render_program_mProjection_location_, 1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
    glBindVertexArray(_partical_vao_);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, sphere_details +2, _partical_count_);

    glBindVertexArray(0);
}