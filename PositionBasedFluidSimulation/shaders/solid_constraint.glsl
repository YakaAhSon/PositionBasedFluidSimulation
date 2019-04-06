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


void main(void)
{
    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz;

    pos = pos - vec3(3, -4, 0);


    if (length(pos) >= 2)return;

    ivec3 ipos = ivec3((pos + vec3(2)) * 10);
    if (ipos.x < 0 || ipos.x >= 40 || ipos.y < 0 || ipos.y >= 40 || ipos.z < 0 || ipos.z >= 40)return;

    struct {
        vec3 norm;// plane normal
        int solid;
        vec3 pos;// one point on the plane
        int _padding_;
    } voxel = voxels[ipos.x * 1600 + ipos.y*40 + ipos.z];

    
    vec3 delta = dot(voxel.pos - pos, voxel.norm)*voxel.norm;

    pos_curr[gl_GlobalInvocationID.x].xyz += delta;
}