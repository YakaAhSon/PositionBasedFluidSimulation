#include "solidmodel.h"

#include"utilities.h"

static Voxel voxel[40][40][40];

void SolidModel::voxelize()
{

    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 40; y++) {
            for (int z = 0; z < 40; z++) {
                glm::vec3 pos{ x / 10.0, y / 10.0, z / 10.0 };

                pos = pos - glm::vec3(2, 2, 2);

                float r = glm::length(pos);

                if (r < 2.0&& r>1.5) {
                    voxel[x][y][z].normal = pos / r;
                    voxel[x][y][z].solid = 1;
                    voxel[x][y][z].point = pos / r * 2.0;
                }
                else {
                    voxel[x][y][z].solid = 0;
                }
            }
        }
    }

    glGenBuffers(1, &_voxel_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _voxel_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 40 * 40 * 40 * sizeof(Voxel), voxel, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SolidModel::runConstraint(GLuint partical_pos_buffer, int partical_count)
{
    static GLuint program = util::createProgram_C(util::readFile("shaders\\solid_constraint.glsl"));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partical_pos_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _voxel_buffer_);

    glUseProgram(program);

    glDispatchCompute(partical_count / 128, 1, 1);
}
