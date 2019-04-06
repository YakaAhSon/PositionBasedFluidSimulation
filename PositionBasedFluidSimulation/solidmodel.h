#pragma once

#ifndef __PBF_SOLID_MODEL_H__

#define __OBF_SOLID_MODEL_H__

#include"predef.h"

#include"camera.h"

#include<vector>

// data structure in GPU
#pragma pack(16)
typedef struct Voxel{
    
    glm::vec3 normal;// plane normal
    int solid;
    glm::vec3 point;// one point on the plane

    int _padding_;

}Voxel;


#pragma pack(4)
typedef struct ModelVertexData {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 tex;
}ModelVertexData;


class SolidModel {
private:

    GLuint _voxel_buffer_;

    GLuint _vao_;
    GLuint _vbo_;

    std::vector<ModelVertexData> _mesh_;

public:

    void voxelize();

    void runConstraint(GLuint partical_pos_buffer, int partical_count);

    void render(Camera& camera);

    void loadModel(const std::string& filename);

};

#endif