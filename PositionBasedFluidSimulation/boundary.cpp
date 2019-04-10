#include"boundary.h"

#include"utilities.h"

#include"vector"

glm::vec3 boundary::bmin = glm::vec3(-11.5, -5.5, -5.5);
glm::vec3 boundary::bmax = glm::vec3(11.5, 5.5, 5.5);

static bool wave=false;

void boundary::update()
{

    static float timer = 0;

    if (wave) {

        bmin.x = -10.0 - glm::cos(timer)*1.5;
        timer += 0.05;
    }
    else {
        timer = 0;
        bmin.x = -11.5;
    }
}

void boundary::toggleWave()
{
    wave = !wave;
}

void boundary::render(Camera& camera)
{
    static GLuint program = []() {
        GLuint p = util::createProgram_VF(util::readFile("shaders\\mesh_render_vertex.glsl"),
            util::readFile("shaders\\mesh_render_fragment.glsl"));
        glBindAttribLocation(program, 0, "vVertex");
        glBindAttribLocation(program, 1, "vNorm");

        util::linkProgram(p);
        return p;
    }();
    bindUniformLocation(mMVP);
    bindUniformLocation(mModelRot);
    bindUniformLocation(color);
    
    using Vertex = struct {
        glm::vec3 vVertex;
        glm::vec3 vNormal;
    };
    static std::vector<Vertex> vertices(6 * 2 * 3);
    vertices.clear();
    auto addTriangle=[&](glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 normal) {
        vertices.push_back({ v1, normal });
        vertices.push_back({ v2, normal });
        vertices.push_back({ v3, normal });
    };
    
    auto addFace = [&](glm::vec3 orig, glm::vec3 x, glm::vec3 y, glm::vec3 norm) {
        addTriangle(orig, orig + y, orig + x, norm);
        addTriangle(orig + x, orig + y, orig + x + y, norm);
    };
    static GLuint vao, vbo;

    
    static bool onLoad= []() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*6*2*3, NULL, GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);


        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*4));
        return true;
    }();
    addFace(glm::vec3(-11.5, 5.5, -5.5), glm::vec3(23, 0, 0), glm::vec3(0, -11, 0), glm::vec3(0, 0, 1));
    addFace(glm::vec3(-11.5, -5.5, -5.5), glm::vec3(23, 0, 0), glm::vec3(0, 0, 11), glm::vec3(0, 1, 0));
    addFace(glm::vec3(11.5, 5.5, -5.5), glm::vec3(0, 0, 11), glm::vec3(0, -11, 0), glm::vec3(1, 0, 0));

    addFace(glm::vec3(bmin.x, 5.5, 5.5), glm::vec3(0, 0, -11), glm::vec3(0, -11, 0), glm::vec3(1, 0, 0));

    glUseProgram(program);
    glUniformMatrix4fv(mMVP, 1, GL_FALSE, &camera.getViewProjectionMatrix()[0][0]);

    glm::mat3 m = glm::identity<glm::mat3>();

    glUniformMatrix3fv(mModelRot, 1, GL_FALSE, &m[0][0]);

    float colors[3] = { 0.5,0.9,0.7 };
    glUniform3fv(color, 1, colors);

    glBindVertexArray(vao);
    glNamedBufferSubData(vbo, 0, sizeof(Vertex)*vertices.size(), &vertices[0]);


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

}
