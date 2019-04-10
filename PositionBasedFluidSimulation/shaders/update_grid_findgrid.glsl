#version 450

// compute shader

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer partical_buffer {
    struct {
        vec3 pos;
        float lambda;
        vec3 pos_prev;

        uint grid_idx;

        vec3 delta_p;

        uint idx_in_grid;
    }particals[];
};

layout(std430, binding = 1) buffer grid_buffer {
    struct {
        uint count;
        uint start;
    } grids[];
};

uniform float cellsize;

uniform int grid_edge_count;
uniform int grid_edge_count2;


uint getGridIdx(vec3 v) {
    ivec3 iv = ivec3((v + vec3(12, 6, 6)) / cellsize);

    iv = max(iv, ivec3(0, 0, 0));
    iv = min(iv, ivec3(grid_edge_count*2 - 1,grid_edge_count-1,grid_edge_count-1));

    return iv.z*grid_edge_count2*2 + iv.y*grid_edge_count*2 + iv.x;
}

void main(void)
{
    uint idx = getGridIdx(particals[gl_GlobalInvocationID.x].pos);
    particals[gl_GlobalInvocationID.x].grid_idx = idx;

    particals[gl_GlobalInvocationID.x].idx_in_grid = atomicAdd(grids[idx].count, uint(1));

}