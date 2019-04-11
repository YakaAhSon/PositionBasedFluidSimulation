#include "solidmodel.h"

#include"utilities.h"
#include "cloth.h"
#include"boundary.h"


void SolidModel::predict()
{
    glm::vec3 tmp = _COM_;
    _COM_ = _COM_ * 2 - _COM_prev_ - glm::vec3(0, 9.8*0.5*(1 / 60.0)*(1 / 60.0), 0);
    _COM_prev_ = tmp;

    glm::quat orientation_delta = _orientation_ * glm::inverse(_orientation_prev_);
    glm::quat iq = glm::identity<glm::quat>();
    orientation_delta = orientation_delta * 0.9f + iq * 0.1f;

    glm::quat tmpquat = _orientation_;

    _orientation_ = orientation_delta * _orientation_;
    _orientation_ = glm::normalize(_orientation_);

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

void SolidModel::predictAll(void* c) {
    for (auto m : _unfixed_models_) {
        m->predict();
        m->collisionWithCloth(c);
    }

    int size = _unfixed_models_.size();

    if (size < 2)
        return;

    for (int i=0;i<size;i++)
    {
        SolidModel* m1 = _unfixed_models_[i];
        for (int j = 0; j < size; j++)
        {
            if (i == j)continue;
            SolidModel* m2 = _unfixed_models_[j];
            m1->solveCollision(m2);
        }
    }

    
}

void SolidModel::renderAll(Camera & camera)
{
    for (auto m : _unfixed_models_) {
        m->render(camera);
    }
    for (auto m : _fixed_models_) {
        m->render(camera);
    }
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

void SolidModel::collisionWithCloth(void* c)
{
    Cloth* cloth = (Cloth*)c;

    std::vector<Vertex>& vertices = cloth->getVertices();

    for (auto& v : vertices)
    {
        glm::vec3 pos = v.pos;
        glm::vec3 pos_local = this->getLocalPos(pos);

        int bFixed = v.fixed;

        Voxel* voxel = getVoxelLocal(pos_local);
        if (voxel == NULL || !voxel->solid)continue;

        glm::vec3 dir = voxel->pos - pos_local;
        float l = glm::length(dir);
        glm::vec3 norm = voxel->norm;
        float depth = glm::dot(dir, norm);

        if (depth < 0.0f)
        {
            continue;
        }

        float depth1, depth2;
        // not correct simulation, just for visualization
        if (bFixed)
        {
            depth1 = depth;
            depth2 = 0.0f;
        }
        else
        {
            depth1 = depth*0.3;
            depth2 = depth * 0.7;
        }

        this->positionImpulse(v.pos, -norm, depth1);

        glm::vec3 norm_g = glm::rotate(_orientation_, norm);

        v.pos += norm_g * depth2;
    }
    
}
// m.vertices vs self.voxel
void SolidModel::solveCollision(SolidModel * m)
{
    for (const ModelVertexData & v : m->_mesh_) {
        glm::vec3 pos_other_g = m->getGlobalPos(v.pos);
        glm::vec3 pos_self_l = this->getLocalPos(pos_other_g);

        Voxel* voxel = getVoxelLocal(pos_self_l);
        if (voxel == NULL || !voxel->solid)continue;

        glm::vec3 dir = voxel->pos - pos_self_l;
        float l = glm::length(dir);
        glm::vec3 norm = voxel->norm;
        float depth = glm::dot(dir, norm);

        if (depth<0.0f)
        {
            continue;
        }

        float totalM = this->getMass() + m->getMass();

        float depth1 = depth * m->getMass() / totalM;
        float depth2 = depth * this->getMass() / totalM;

        glm::vec3 norm_g = glm::rotate(_orientation_, norm);

        glm::quat invO = glm::inverse(m->_orientation_);
        glm::vec3 norm_other_l = glm::rotate(invO, norm_g);

        this->positionImpulse(v.pos, -norm, depth1);
        m->positionImpulse(pos_self_l, norm_other_l, depth2);
    }
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

void SolidModel::moveGlobal(glm::vec3 p) {
    _COM_ += p;
    _COM_prev_ += p;
}