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

    static constexpr float _voxel_size_ = 0.05;

    GLuint _voxel_buffer_;
    glm::ivec3 _voxel_space_size_;

#pragma pack(push)
#pragma pack(16)
    using FluidImpulse = struct {
        glm::vec3 pos;
        float depth;
        glm::vec3 normal;
        float _padding;
    };
#pragma pack(pop)

    static constexpr int _max_impulses_ = 64*1024;
    static GLuint _impulse_counter_buffer_;
    static GLuint _impulse_buffer_;

    GLuint _vao_;
    GLuint _vbo_;

    // bounding box
    glm::vec3 _bmin_;
    glm::vec3 _bmax_;

    std::vector<ModelVertexData> _mesh_;
private:
    // dynamics
    bool _fixed_;
    glm::vec3 _COM_local_;// center of mass in local coordinate
    glm::vec3 _COM_;// center of mass in global coordinate

    const glm::vec3 centerOfGeometry() const { return _COM_ - _COM_local_; }

    glm::quat _orientation_;

    glm::vec3 _COM_prev_;
    glm::quat _orientation_prev_;

    glm::mat4 _mModel_;
    glm::mat3 _mModelRot_;
    glm::mat4 _mView_;

    float _mass_;
    glm::mat3 _inverse_inertia_tensor_;
    glm::mat3 _inertia_tensor_;
    glm::mat3 _inertia_tensor_vmass_;// inertia tensor divided by vertex mass (assume vertex mass = 1)

public:
    // dynamics
    void predict();
    // pos and norm are in local coordinate!
    void positionImpulse(glm::vec3 pos, glm::vec3 norm, float depth);

    void setMass(float mass);
    void updateModelViewMatrices();

    const glm::mat4& getMModel()const { return _mModel_; }
    const glm::mat4& getMView()const { return _mView_; }


public:

    void voxelize();

    void runConstraint(GLuint partical_pos_buffer, int partical_count);

    void render(Camera& camera);

    void loadModel(const std::string& filename);

};

#endif