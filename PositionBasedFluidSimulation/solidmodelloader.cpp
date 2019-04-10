#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "solidmodel.h"

#include"utilities.h"

using namespace glm;



SolidModel::SolidModel(const std::string& filename) {
    
    Assimp::Importer* importer = new Assimp::Importer();

    importer->ReadFile(filename, aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    const aiScene *scene = importer->GetScene();
    

    for (unsigned int meshI = 0; meshI < scene->mNumMeshes; ++meshI)
    {
        aiMesh *mesh = scene->mMeshes[meshI];
        aiFace *faces = mesh->mFaces;

        for (unsigned int faceI = 0; faceI < mesh->mNumFaces; ++faceI)
        {
            vec3 vertices[3];
            vec3 normals[3];
            vec2 texCoords[3];

            aiVector3D &a = mesh->mVertices[faces[faceI].mIndices[0]];
            aiVector3D &b = mesh->mVertices[faces[faceI].mIndices[1]];
            aiVector3D &c = mesh->mVertices[faces[faceI].mIndices[2]];
            vertices[0] = vec3(a.x, a.y, a.z);
            vertices[1] = vec3(b.x, b.y, b.z);
            vertices[2] = vec3(c.x, c.y, c.z);

            aiVector3D &na = mesh->mNormals[faces[faceI].mIndices[0]];
            aiVector3D &nb = mesh->mNormals[faces[faceI].mIndices[1]];
            aiVector3D &nc = mesh->mNormals[faces[faceI].mIndices[2]];
            normals[0] = vec3(na.x, na.y, na.z);
            normals[1] = vec3(nb.x, nb.y, nb.z);
            normals[2] = vec3(nc.x, nc.y, nc.z);

            if (mesh->mTextureCoords[0] != NULL)
            {
                aiVector3D &ta = mesh->mTextureCoords[0][faces[faceI].mIndices[0]];
                aiVector3D &tb = mesh->mTextureCoords[0][faces[faceI].mIndices[1]];
                aiVector3D &tc = mesh->mTextureCoords[0][faces[faceI].mIndices[2]];

                texCoords[0] = vec2(ta.x, ta.y);
                texCoords[1] = vec2(tb.x, tb.y);
                texCoords[2] = vec2(tc.x, tc.y);
            }

            for (int i = 0; i < 3; i++) {
                _mesh_.push_back({ vertices[i],normals[i],texCoords[i] });
            }
        }
    }


    // calculate bounding box
    _bmin_ = glm::vec3(100000.0);
    _bmax_ = glm::vec3(-100000.0);
    for (auto& vertex : _mesh_) {
        _bmin_ = glm::min(_bmin_, vertex.pos);
        _bmax_ = glm::max(_bmax_, vertex.pos);
    }

    // refine center, so that the center of the mesh is always (0,0,0)
    glm::vec3 center = (_bmin_ + _bmax_)*0.5;
    for (auto& vertex : _mesh_) {
        vertex.pos = vertex.pos - center;
    }

    // refine bounding box, slightly larger than the object
    _bmin_ = _bmin_ - center - vec3(_voxel_size_/2);
    _bmax_ = _bmax_ - center + vec3(_voxel_size_/2);

    _voxel_space_size_ = glm::ivec3((_bmax_ - _bmin_) / _voxel_size_);

    _voxels_.resize(_voxel_space_size_.x*_voxel_space_size_.y*_voxel_space_size_.z);

    // init pos and orientation
    _COM_local_ = _COM_prev_ = glm::vec3(0);

    _orientation_ = _orientation_prev_ = glm::quat_identity<float,glm::qualifier::highp>();

    // init inertia tensor
    float Ixx = 0;
    float Iyy = 0;
    float Izz = 0;
    float Ixy = 0;
    float Iyz = 0;
    float Izx = 0;

    for (const auto& v : _mesh_) {
        float x, y, z;
        x = v.pos.x - _COM_local_.x;
        y = v.pos.y - _COM_local_.y;
        z = v.pos.z - _COM_local_.z;
        Ixx += y * y + z * z;
        Iyy += x * x + z * z;
        Izz += x * x + y * y;
        Ixy -= x * y;
        Iyz -= y * z;
        Izx -= z * x;
    }
    _inertia_tensor_vmass_ = {
        {Ixx,Ixy,Izx},
        {Ixy,Iyy,Iyz},
        {Izx,Iyz,Izz}
    };
    
    glGenVertexArrays(1, &_vao_);
    glBindVertexArray(_vao_);
    glGenBuffers(1, &_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo_);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ModelVertexData)*_mesh_.size(), &_mesh_[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertexData), (void*)offsetof(ModelVertexData, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertexData), (void*)offsetof(ModelVertexData, norm));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertexData), (void*)offsetof(ModelVertexData, tex));


}

std::vector<SolidModel*> SolidModel::_fixed_models_ = {};
std::vector<SolidModel*> SolidModel::_unfixed_models_ = {};

SolidModel* SolidModel::loadModel(const std::string& filename, bool fixed, float mass) {
    SolidModel* model = new SolidModel(filename);
    model->voxelize();
    model->_fixed_ = fixed;
    model->setMass(mass);
    if (fixed) {
        _fixed_models_.push_back(model);
    }
    else {
        _unfixed_models_.push_back(model);
    }
    return model;
}