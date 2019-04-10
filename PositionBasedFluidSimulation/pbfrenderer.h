#pragma once
#ifndef __PBF_RENDERER__
#define __PBF_RENDERER__

#include<glad/glad.h>

#include"camera.h"

#include"pbfsim.h"

class PBFRenderer
{
private:
    GLuint _sphere_vertices_buffer_;
    GLuint _render_program_;

    GLuint _render_program_mView_location_;
    GLuint _render_program_mProjection_location_;

    
    GLuint _partical_vao_;

    GLuint* _p_vbo_;

    int _partical_count_;

    const PBF* _pbf_;

public:
    Camera camera;

public:
    void initialize(const PBF* pbf, int partical_count);

    void render();
};

#endif