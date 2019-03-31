#version 450

// compute shader

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec3 pos_curr[];
};

layout(std430, binding = 1) buffer grid_partical_idx_buffer {
    uint grid_partical_idx[];
};

uniform float grid_size;
uniform int grid_edge_count;
uniform int grid_edge_count2;

int getGridIdx(vec3 v) {
    ivec3 iv = ivec3((v + vec3(5, 5, 5)) / grid_size);

    iv = max(iv, ivec3(0, 0, 0));
    iv = min(iv, ivec3(grid_edge_count - 1));

    return iv.z*grid_edge_count2 + iv.y*grid_edge_count + iv.x;
}

void main(void)
{
    int grid_idx = getGridIdx(pos_curr[gl_GlobalInvocationID.x]);
    grid_partical_idx[grid_idx] = gl_GlobalInvocationID.x;
}