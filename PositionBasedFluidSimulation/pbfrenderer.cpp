#include"utilities.h"
#include<vector>
#include"predef.h"
#include"pbfrenderer.h"


static const int sphere_details = 8;
static const glm::vec2 blurDirX = glm::vec2(1.0f / 1024.0f, 0.0f);
static const glm::vec2 blurDirY = glm::vec2(0.0f, 1.0f / 1024.0f);
static float filterRadius = 30.0f;

void PBFRenderer::initialize(const PBF* pbf, int partical_count)
{
    _pbf_ = pbf;
    _partical_count_ = partical_count;

    _render_program_ = util::createProgram_VF(
        util::readFile("shaders\\fluid_render_vertex.glsl"),
        util::readFile("shaders\\fluid_render_fragment.glsl"));

    glBindAttribLocation(_render_program_, 0, "vVertex");
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
    glBindVertexArray(0);

// FBO
    _depth.init(1024, 1024);
    _blur.init(1024, 1024);
    _normal.init(1024, 1024);
    
// Screen shader
    _screen_program_ = util::createProgram_VF(
        util::readFile("shaders\\framebuffers_screen.vs"),
        util::readFile("shaders\\framebuffers_screen.fs"));

    _blur_program_ = util::createProgram_VF(
        util::readFile("shaders\\blur.vs"),
        util::readFile("shaders\\blur.fs"));
/*
    _normal_program_ = util::createProgram_VF(
        util::readFile("shaders\\normal.vs"),
        util::readFile("shaders\\normal.fs"));
        */
}

void PBFRenderer::render()
{
    int screensize[4];
    glGetIntegerv(GL_VIEWPORT, screensize);

    glViewport(0, 0, 1024, 1024);

    static const GLfloat g_quad_vertex_buffer_data[18] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    static GLuint quad_vertexbuffer;
    static GLuint vao = [&]() {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);


        glGenBuffers(1, &quad_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * 4, 0);

        return vao;
    }();

// Particle Depth
    glBindFramebuffer(GL_FRAMEBUFFER, _depth._framebufferName);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _depth._framebufferName);
    
    glUseProgram(_render_program_);

    glUniformMatrix4fv(_render_program_mView_location_, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
    glUniformMatrix4fv(_render_program_mProjection_location_, 1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
    glBindVertexArray(_partical_vao_);

    GLuint partical_buffer = _pbf_->getCurrPosVBO();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partical_buffer);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, sphere_details +2, _partical_count_);
    glBindVertexArray(0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

// Blur Depth
    glBindFramebuffer(GL_FRAMEBUFFER, _blur._framebufferName);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _blur._framebufferName);

    glUseProgram(_blur_program_);

    glBindVertexArray(vao);
    glUniform1i(glGetUniformLocation(_blur_program_, "depthTexture"), 0);
    glUniform2f(glGetUniformLocation(_blur_program_, "blurDir"), blurDirY.x, blurDirY.y);
    glUniform1f(glGetUniformLocation(_blur_program_, "filterRadius"), filterRadius);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, _depth._renderedTexture);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);


    glBindVertexArray(0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Compute Normal

    static auto compute_normal = [&]() {
        GLuint p = util::createProgram_C(util::readFile("shaders\\fluid_normal.glsl"));
        GLuint dTex = glGetUniformLocation(p, "depthTexture");
        GLuint tanFov = glGetUniformLocation(p, "tanfov");

        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1024 * 1024, NULL, GL_STATIC_READ);

        struct {
            GLuint program;
            GLuint dTex;
            GLuint tanfov;
            GLuint buffer;
        }result;

        result.program = p;
        result.dTex = dTex;
        result.tanfov = tanFov;
        result.buffer = buffer;
        return result;
    }();

    glBindTexture(GL_TEXTURE_RECTANGLE, _blur._renderedTexture);
    glUseProgram(compute_normal.program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, compute_normal.buffer);
    glUniform1i(compute_normal.dTex, 0);
    glUniform2fv(compute_normal.tanfov, 1, &camera.getTanFov()[0]);

    glDispatchCompute(32, 32, 1);
    glFinish();

    glUseProgram(_screen_program_);
    glBindVertexArray(vao);
    glUniform1i(glGetUniformLocation(_screen_program_, "depthTexture"), 0);

    glUniformMatrix4fv(glGetUniformLocation(_screen_program_, "mProjection"), 1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);

    glUniform2fv(glGetUniformLocation(_screen_program_, "tanfov"), 1, &camera.getTanFov()[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, _depth._renderedTexture);



    glDisable(GL_CULL_FACE);

    glViewport(screensize[0], screensize[1], screensize[2], screensize[3]);
    glEnable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    

}

void fboWrapper::init(int nWidth, int nHeight)
{
    glGenFramebuffers(1, &_framebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferName);

    glGenTextures(1, &_renderedTexture);

    glBindTexture(GL_TEXTURE_RECTANGLE, _renderedTexture);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, nWidth, nHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _renderedTexture, 0);
    
    glGenRenderbuffers(1, &_depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthrenderbuffer);

    std::cout << glCheckFramebufferStatus(GL_FRAMEBUFFER)<<std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
