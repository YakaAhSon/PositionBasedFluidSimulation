#version 450

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;


layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};

layout(std430, binding = 1) buffer voxel_buffer {
    struct{
        vec3 norm;// plane normal
        int solid;
        vec3 pos;// one point on the plane

        int _padding_;
    }voxels[];
};

uniform vec3 bBoxMin;
uniform ivec3 voxelSpaceSize;
uniform float voxelSize;


void main(void)
{
    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz + vec3(0, 2, 0);


    ivec3 ipos = ivec3((pos - bBoxMin)/ voxelSize);

    if (any(lessThan(ipos, ivec3(0))) || any(greaterThanEqual(ipos, voxelSpaceSize)))return;

    struct {
        vec3 norm;// plane normal
        int solid;
        vec3 pos;// one point on the plane
        int _padding_;
    } voxel = voxels[ipos.x * voxelSpaceSize.y*voxelSpaceSize.z + ipos.y*voxelSpaceSize.z + ipos.z];

    
    vec3 delta = dot(voxel.pos - pos, voxel.norm)*voxel.norm;

    pos_curr[gl_GlobalInvocationID.x].xyz += delta;
}