#include "cloth.h"

#include "utilities.h"

void Cloth::updateBoundingBox()
{
    _bmin_ = glm::vec3(10000.0);
    _bmax_ = glm::vec3(-10000.0);
    for (Vertex&v : _vertices_) {
        _bmin_ = glm::min(_bmin_, v.pos - glm::vec3(_vertex_size_));
        _bmax_ = glm::max(_bmax_, v.pos + glm::vec3(_vertex_size_));
    }
}

Cloth::Cloth(int gridX, int gridY, glm::vec3 upperLeft, glm::vec3 upperRight, glm::vec3 lowerLeft):
    _grid_x_(gridX),
    _grid_y_(gridY)
{
    _vertices_.resize((gridX+1)*(gridY+1));

    // init vertices and upload to GPU
    glm::vec3 stepX = (upperRight - upperLeft)/gridX;
    glm::vec3 stepY = (lowerLeft - upperLeft)/gridY;

    _vertex_size_ = glm::length(stepX);

    for (int y = 0; y <= gridY; y++) {
        for (int x = 0; x <= gridX; x++) {
            Vertex& v = getVertex(x, y);
            v.pos = v.pos_prev = upperLeft + stepX * x + stepY * y;
            v.fixed = 0;
        }
    }

    glGenBuffers(1, &_vertices_buffer_);
    glNamedBufferData(_vertices_buffer_, _vertices_.size() * sizeof(Vertex), &_vertices_[0], GL_STREAM_DRAW);

    // set pins
    getVertex(0, 0).fixed = 1;
    getVertex(gridX, 0).fixed = 1;
    getVertex(0, gridY).fixed = 1;
    getVertex(gridX, gridY).fixed = 1;

    // constraints in X
    for (int x = 0; x < gridX; x++) {
        for (int y = 0; y <= gridY; y++) {
            Vertex& v1 = getVertex(x, y);
            Vertex& v2 = getVertex(x+1, y);
            constraints.push_back({
                &v1,
                &v2,
                glm::length(v1.pos - v2.pos)*1.1f
                });
        }
    }
    // constraints in Y
    for (int x = 0; x <= gridX; x++) {
        for (int y = 0; y < gridY; y++) {
            Vertex& v1 = getVertex(x, y);
            Vertex& v2 = getVertex(x, y + 1);
            constraints.push_back({
                &v1,
                &v2,
                glm::length(v1.pos - v2.pos)
                });
        }
    }
    
}

void Cloth::predict()
{
    // predict pos
    for (Vertex& v : _vertices_) {
        if (v.fixed)continue;
        glm::vec3 tmp = v.pos;
        v.pos = v.pos * 2 - v.pos_prev + glm::vec3(0.0, -0.00136, 0);
        v.pos_prev = tmp;
    }
    // solve constraints
    constexpr int steps = 3;
    for(int i = 0;i<steps;i++)
    for (Constraint& c : constraints) {
        glm::vec3 norm = c.v1->pos - c.v2->pos;
        float l = glm::length(norm);
        if (l < c.rest_dis)continue;
        norm = norm / l;
        l = l - c.rest_dis;
        norm = l * norm * 0.5;
        if (c.v1->fixed&&c.v2->fixed) {
            continue;
        }
        if(c.v1->fixed)
            c.v2->pos += 2*norm;
        else if (c.v2->fixed) {
            c.v1->pos -= 2 * norm;
        }
        else {

            c.v1->pos -= norm;
            c.v2->pos += norm;
        }
    }
    // update buffer
    glNamedBufferSubData(_vertices_buffer_, 0, sizeof(Vertex)*_vertices_.size(), &_vertices_[0]);

    updateBoundingBox();

}

void Cloth::blowByFluid(GLuint fluid_buffer, int partical_count)
{
    static GLuint program = util::createProgram_C(util::readFile("shaders\\blow_cloth.glsl"));
    bindUniformLocation(bMin);
    bindUniformLocation(bMax);

    using Impulse = struct {
        glm::vec3 impulse;
        int padding1;
        glm::vec3 pos;
        int padding2;
    };

    static GLuint impulse_buffer = []() {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Impulse)*8*1024, NULL, GL_DYNAMIC_READ);
        return buffer;
    }();

    static GLuint impulse_counter_buffer = []() {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buffer);
        GLuint zero = 0;
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, &zero, GL_DYNAMIC_READ);
        return buffer;
    }();

    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fluid_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, impulse_buffer);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, impulse_counter_buffer);

    glUniform3fv(bMin, 1, &_bmin_.x);
    glUniform3fv(bMax, 1, &_bmax_.x);

    glDispatchCompute(partical_count / 128 / 32, 1, 1);

    GLuint* counter_pointer = (GLuint*)glMapNamedBuffer(impulse_counter_buffer, GL_READ_WRITE);
    GLuint counter = counter_pointer[0];
    counter_pointer[0] = 0;
    glUnmapNamedBuffer(impulse_counter_buffer);

    static std::vector<Impulse> impulses(32 * 1024);
    glGetNamedBufferSubData(impulse_buffer, 0, counter * sizeof(Impulse), &impulses[0]);

    int step = counter / 100;
    step = glm::max(step, 1);
    for (int i = 0; i < counter; i++) {
        Impulse& impulse = impulses[i];
        for (Vertex& v : _vertices_) {
            if (glm::length2(impulse.pos - v.pos) < 4*_vertex_size_*_vertex_size_) {
                v.pos += impulse.impulse*0.001*step;
            }
        }
    }

}

void Cloth::render(Camera & camera)
{
    static GLuint program, vao, vbo, mMVPLocation;
    static bool onLoad = [&]() {
        program = util::createProgram_VF(
            util::readFile("shaders\\cloth_render_vertex.glsl"),
            util::readFile("shaders\\cloth_render_fragment.glsl"));
        glBindAttribLocation(program, 0, "vVertex");
        glBindAttribLocation(program, 1, "vNorm");
        glBindAttribLocation(program, 2, "vTexCoord");
        glLinkProgram(program);

        mMVPLocation = glGetUniformLocation(program, "mMV");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVertex)*_grid_x_*_grid_y_ * 2 * 3, NULL, GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)offsetof(TriangleVertex, vVertex));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)offsetof(TriangleVertex, vNorm));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)offsetof(TriangleVertex, vTexCoord));

        return true;
    }();


    // construct triangles
    static std::vector<TriangleVertex> _triangles_;
    _triangles_.clear();
    for (int x = 0; x < _grid_x_; x++) {
        for (int y = 0; y < _grid_y_; y++) {

            auto addTriangle = [&](Vertex& v1, Vertex& v2, Vertex& v3) {
                glm::vec3 norm = glm::cross((v2.pos - v1.pos), (v3.pos - v1.pos));
                if (glm::length2(norm) > 0.00001)norm = glm::normalize(norm);

                glm::vec2 vTexCoord = glm::vec2(float(x) / float(_grid_x_), float(y) / float(_grid_y_));

                _triangles_.push_back({ v1.pos,norm,vTexCoord });
                _triangles_.push_back({ v2.pos,norm,vTexCoord });
                _triangles_.push_back({ v3.pos,norm,vTexCoord });
                return;
            };

            addTriangle(getVertex(x, y), getVertex(x, y+1), getVertex(x+1, y));
            addTriangle(getVertex(x+1, y+1), getVertex(x, y+1), getVertex(x+1, y));
        }
    }

    glNamedBufferSubData(vbo, 0, sizeof(TriangleVertex)*_triangles_.size(), &_triangles_[0]);

    glUseProgram(program);

    glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, &camera.getViewProjectionMatrix()[0][0]);

    glBindVertexArray(vao);

    glDisable(GL_CULL_FACE);

    glDrawArrays(GL_TRIANGLES, 0, _triangles_.size());

    glEnable(GL_CULL_FACE);

}
