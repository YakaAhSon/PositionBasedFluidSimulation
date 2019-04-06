#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "solidmodel.h"

#include"utilities.h"

using namespace glm;

void SolidModel::loadModel(const std::string& filename) {
    
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