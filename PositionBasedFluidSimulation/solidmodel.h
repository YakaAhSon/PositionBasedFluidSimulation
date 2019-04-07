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

    static constexpr float _voxel_size_ = 0.1;

    GLuint _voxel_buffer_;
    glm::ivec3 _voxel_space_size_;

    GLuint _vao_;
    GLuint _vbo_;

    // bounding box
    glm::vec3 _bmin_;
    glm::vec3 _bmax_;

    std::vector<ModelVertexData> _mesh_;
private:
    // dynamics
    bool _fixed_;
    glm::vec3 _COM_;
    glm::quat _orientation_;

    glm::vec3 _velocity_;
    glm::vec3 _anglar_velocity_;
public:
    // dynamics
    void predict();
    void updateVelocity();

public:

    void voxelize();

    void runConstraint(GLuint partical_pos_buffer, int partical_count);

    void render(Camera& camera);

    void loadModel(const std::string& filename);

};

#endif