#include "solidmodel.h"

#include"utilities.h"




void SolidModel::predict()
{
    glm::vec3 tmp = _COM_;
    _COM_ = _COM_ * 2 - _COM_prev_ -glm::vec3(0,9.8*0.5*(1 / 60.0)*(1 / 60.0), 9.8*0.5*(1 / 60.0)*(1 / 60.0));
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

        if (pos.x < -11.5) {
            positionImpulse(pos, glm::vec3(1, 0, 0), -11.5 - pos.x);
        }
        if (pos.x >11.5) {
            positionImpulse(pos, glm::vec3(-1, 0, 0), pos.x - 11.5);
        }
        if (pos.y < -5.5) {
            positionImpulse(pos, glm::vec3(0, 1, 0), -5.5 - pos.y);
        }
        if (pos.y > 5.5) {
            positionImpulse(pos, glm::vec3(0, -1, 0), pos.y - 5.5);
        }
        if (pos.z < -5.5) {
            positionImpulse(pos, glm::vec3(0, 0, 1), -5.5 - pos.z);
        }
        if (pos.z > 5.5) {
            positionImpulse(pos, glm::vec3(0, 0, -1), pos.z - 5.5);
        }
    }
    //t.toc("RigidBodu=y Simulation Time");
}

void SolidModel::positionImpulse(glm::vec3 pos, glm::vec3 norm, float depth)
{
    glm::quat inverse_rot = glm::inverse(_orientation_);
    pos = glm::rotate(inverse_rot, pos - _COM_);
    norm = glm::rotate(inverse_rot, norm);

    glm::vec3 r_norm = glm::cross(pos, norm);

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
