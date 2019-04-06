#pragma once

#ifndef __PBF_SOLID_MODEL_H__

#define __OBF_SOLID_MODEL_H__

#include"predef.h"

#include<vector>

// data structure in GPU
#pragma pack(16)
typedef  struct Voxel{
    
    glm::vec3 normal;// plane normal
    int solid;
    glm::vec3 point;// one point on the plane

    int _padding_;

}Voxel;

class SolidModel {
private:

    
    GLuint _voxel_buffer_;


public:

    void voxelize();

    void runConstraint(GLuint partical_pos_buffer, int partical_count);
};

class SolidBox {

};

#endif