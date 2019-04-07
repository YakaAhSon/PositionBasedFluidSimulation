#include "solidmodel.h"

#include"utilities.h"

#include"boundary.h"


void SolidModel::predict()
{
    glm::vec3 tmp = _COM_;
    _COM_ = _COM_ * 2 - _COM_prev_ - glm::vec3(0, 9.8*0.5*(1 / 60.0)*(1 / 60.0), 0);
    _COM_prev_ = tmp;

    glm::quat orientation_delta = _orientation_ * glm::inverse(_orientation_prev_);

    glm::quat tmpquat = _orientation_;

    _orientation_ = orientation_delta * _orientation_;
    glm::normalize(_orientation_);

    _orientation_prev_ = tmpquat;

    // solve constraint with boundary
    //util::Timer t;
    int step = glm::max(int(_mesh_.size() / 500),1);
    for(int i = 0;i<_mesh_.size();i+=step){


        const ModelVertexData& v = _mesh_[i];
        glm::vec3 pos = glm::rotate(_orientation_, v.pos) + _COM_;

        if (pos.x < boundary::bmin.x) {
            glm::vec3 norm = glm::vec3(1, 0, 0);
            glm::quat inverse_orien = glm::conjugate(_orientation_);
            norm = glm::rotate(inverse_orien, norm);
            positionImpulse(v.pos, norm, boundary::bmin.x - pos.x);
        }
        if (pos.x > boundary::bmax.x) {
            glm::vec3 norm = glm::vec3(-1, 0, 0);
            glm::quat inverse_orien = glm::conjugate(_orientation_);
            norm = glm::rotate(inverse_orien, norm);
            positionImpulse(v.pos, norm, pos.x - boundary::bmax.x);
        }
        if (pos.y < boundary::bmin.y) {
            glm::vec3 norm = glm::vec3(0, 1, 0);
            glm::quat inverse_orien = glm::conjugate(_orientation_);
            norm = glm::rotate(inverse_orien, norm);
            positionImpulse(v.pos, norm, boundary::bmin.y - pos.y);
        }
        if (pos.y > boundary::bmax.y) {
            glm::vec3 norm = glm::vec3(0, -1, 0);
            glm::quat inverse_orien = glm::conjugate(_orientation_);
            norm = glm::rotate(inverse_orien, norm);
            positionImpulse(v.pos, norm, pos.y - boundary::bmax.y);
        }
        if (pos.z < boundary::bmin.z) {
            glm::vec3 norm = glm::vec3(0, 0, 1);
            glm::quat inverse_orien = glm::conjugate(_orientation_);
            norm = glm::rotate(inverse_orien, norm);
            positionImpulse(v.pos, norm, boundary::bmin.z - pos.z);
        }
        if (pos.z > boundary::bmax.z) {
            glm::vec3 norm = glm::vec3(0, 0, -1);
            glm::quat inverse_orien = glm::conjugate(_orientation_);
            norm = glm::rotate(inverse_orien, norm);
            positionImpulse(v.pos, norm, pos.z - boundary::bmax.z);
        }
    }
    //t.toc("RigidBodu=y Simulation Time");
}

// position impulse in local space
void SolidModel::positionImpulse(glm::vec3 pos, glm::vec3 norm, float depth)
{
    glm::quat inverse_rot = glm::conjugate(_orientation_);

    glm::vec3 r_norm = glm::cross(pos-_COM_local_, norm);

    float J = depth / (1 / _mass_ + glm::dot(r_norm, _inverse_inertia_tensor_*r_norm));

    glm::vec3 deltaCom = J * norm / _mass_;
    glm::vec3 deltaAngle = J * _inverse_inertia_tensor_*r_norm;

    _COM_ += glm::rotate(_orientation_, deltaCom);

    glm::quat deltaRot = glm::quat_identity<float, glm::qualifier::highp>();
    deltaRot = glm::rotate(deltaRot, deltaAngle);

    _orientation_ = _orientation_*deltaRot;
}

void SolidModel::setMass(float mass)
{
    _mass_ = mass;
    _inertia_tensor_ = _inertia_tensor_vmass_ * (mass / _mesh_.size());
    _inverse_inertia_tensor_ = glm::inverse(_inertia_tensor_);
}

void SolidModel::updateModelViewMatrices()
{
    _mModelRot_ = glm::mat3_cast(_orientation_);
    glm::mat4 mRot = glm::mat4_cast(_orientation_);
    glm::mat4 mTranslation = glm::translate(centerOfGeometry());

    _mModel_ = mTranslation*mRot;
    _mView_ = glm::inverse(_mModel_);

}
