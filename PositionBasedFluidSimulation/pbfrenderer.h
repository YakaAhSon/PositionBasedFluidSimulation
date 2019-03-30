#pragma once
#ifndef __PBF_RENDERER__
#define __PBF_RENDERER__

#include<glad/glad.h>

#include"camera.h"
class PBFRenderer
{
private:
    GLuint _sphere_vertices_buffer_;
    GLuint _render_program_;

    GLuint _render_program_mView_location_;
    GLuint _render_program_mProjection_location_;

    
    GLuint _partical_vao_;

    GLuint _vbo_;

    int _partical_count_;

public:
    Camera camera;

public:
    void initialize(GLuint vbo, int partical_count);

    void render();
};

#endif