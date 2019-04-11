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
    std::vector<ModelVertexData> _mesh_; 
    GLuint _vao_;
    GLuint _vbo_;
    void render(Camera& camera);

    // bounding box
    glm::vec3 _bmin_;
    glm::vec3 _bmax_;

    // voxel
#pragma pack(push)
#pragma pack(16)
    using Voxel = struct {
        glm::vec3 norm;// plane normal
        int solid;
        glm::vec3 pos;// one point on the plane
        int _padding_;
    };
#pragma pack(pop)
    std::vector<Voxel> _voxels_;
    static constexpr float _voxel_size_ = 0.07;
    GLuint _voxel_buffer_;
    glm::ivec3 _voxel_space_size_;

    // Fluid Interaction
#pragma pack(push)
#pragma pack(16)
    using FluidImpulse = struct {
        glm::vec3 pos;
        float depth;
        glm::vec3 normal;
        float _padding;
    };
#pragma pack(pop)
    static constexpr int _max_impulses_ = 32*1024;
    static GLuint _impulse_counter_buffer_;
    static GLuint _impulse_buffer_;
    unsigned int _impulse_offset_;
    unsigned int _impulse_count_;

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

private:
    // loader
    SolidModel(const std::string& filename);
    void voxelize();
    void setMass(float mass);
    static std::vector<SolidModel*> _fixed_models_;
    static std::vector<SolidModel*> _unfixed_models_;

private:
    // dynamics
    void predict();
    void positionImpulseGlobal(glm::vec3 pos, glm::vec3 norm, float depth);
    // m.verticed vs self.voxels
    void solveCollision(SolidModel* m);
public:
    void updateModelViewMatrices();

public:
    const glm::mat4& getMModel()const { return _mModel_; }
    const glm::mat4& getMView()const { return _mView_; }
    const float& getMass()const { return _mass_; }
    glm::vec3 getGlobalPos(glm::vec3 pos);
    glm::vec3 getLocalPos(glm::vec3 pos);
    static const std::vector<SolidModel*>& getModels();
    void collisionWithCloth(void* cloth);
    // pos and norm are in local coordinate!
    void positionImpulse(glm::vec3 pos, glm::vec3 norm, float depth);
    // get voxel in local coordinate
    inline Voxel* getVoxelLocal(const glm::vec3& v){
        glm::ivec3 iv = glm::ivec3((v - _bmin_)/ _voxel_size_);

        if ((iv.x<0 || iv.x >=_voxel_space_size_.x) ||
            (iv.y<0 || iv.y >=_voxel_space_size_.y) || 
            (iv.z<0 || iv.z >=_voxel_space_size_.z))
        {
            return NULL;
        }

        return &(_voxels_[iv.x*_voxel_space_size_.y*_voxel_space_size_.z + iv.y*_voxel_space_size_.z + iv.z]);
    }

public:

    static SolidModel* loadModel(const std::string& filename, bool fixed, float mass);

    static void runConstraintsAll(GLuint partical_pos_buffer, int partical_count);

    static void predictAll(void* c);
    static void renderAll(Camera& camera);

public:
    // position control
    void moveGlobal(glm::vec3 p);

};

#endif