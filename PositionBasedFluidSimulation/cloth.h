#pragma once

#ifndef __PBF_CLOTH__
#define __PBF_CLOTH__

#include<vector>

#include"predef.h"

#include"camera.h"

using Vertex = struct {
    glm::vec3 pos;
    int fixed;
    glm::vec3 pos_prev;
    int padding;
};

class Cloth {
private:
    const int _grid_x_;
    const int _grid_y_;
    
    std::vector<Vertex> _vertices_;
    GLuint _vertices_buffer_;

#pragma pack(4)
    using TriangleVertex = struct {
        glm::vec3 vVertex;
        glm::vec3 vNorm;
        glm::vec2 vTexCoord;
    };
    std::vector<TriangleVertex> _triangle_vertices_;

    const int getVertexIdx(int x, int y) const { return x * (_grid_y_ + 1) + y; }
    Vertex& getVertex(int x, int y) { return _vertices_[getVertexIdx(x, y)]; }

    glm::vec3 _bmin_;
    glm::vec3 _bmax_;
    void updateBoundingBox();

    float _vertex_size_;

    using Constraint = struct {
        Vertex* v1;
        Vertex* v2;
        float rest_dis;
    };
    std::vector<Constraint> constraints;

public:
    Cloth(int gridX, int gridY, glm::vec3 upperLeft, glm::vec3 upperRight, glm::vec3 lowerLeft);

    void predict();

    void blowByFluid(GLuint fluid_buffer, int partical_count);

    void render(Camera& camera);

    std::vector<Vertex>& getVertices() { return _vertices_;}

};

#endif
