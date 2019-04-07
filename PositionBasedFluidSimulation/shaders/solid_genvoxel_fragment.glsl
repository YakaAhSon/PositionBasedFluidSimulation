#version 450

layout(std430, binding = 0) buffer voxel_buffer {
    struct {
        vec3 norm;// plane normal
        int solid;
        vec3 pos;// one point on the plane

        int _padding_;
    }voxels[];
};


in vec3 varyingVVertex;
in vec3 varyingVNormal;
in vec3 surfacePos;

uniform ivec3 voxelSpaceSize;

void main(void) {

    ivec3 ipos = ivec3(varyingVVertex);

    int voxelIndex = ipos.x*voxelSpaceSize.y*voxelSpaceSize.z + ipos.y*voxelSpaceSize.z + ipos.z;

    voxels[voxelIndex].norm = varyingVNormal;
    voxels[voxelIndex].solid = 1;
    voxels[voxelIndex].pos = surfacePos;

    gl_FragColor = vec4(1, 0, 0, 0);
}