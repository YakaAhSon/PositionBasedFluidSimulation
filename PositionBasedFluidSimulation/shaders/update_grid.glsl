#version 450

// compute shader

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec3 pos_curr[];
};

layout(std430, binding = 1) buffer grid_partical_count_buffer {
    uint grid_partical_count[];
};

layout(std430, binding = 2) buffer grid_particals_buffer {
    struct {

        vec3 pos;
        uint index;
    } grid_particals[];
};

layout(std430, binding = 3) buffer partical_grid_buffer {
    uint partical_grid_index[];
};

uniform float cellsize;

uniform int cellmaxparticalcount;
uniform int grid_edge_count;
uniform int grid_edge_count2;


int getGridIdx(vec3 v) {
    ivec3 iv = ivec3((v + vec3(6, 6, 6)) / cellsize);

    iv = max(iv, ivec3(0, 0, 0));
    iv = min(iv, ivec3(grid_edge_count - 1));

    return iv.z*grid_edge_count2 + iv.y*grid_edge_count + iv.x;
}

void main(void)
{
    int grid_idx = getGridIdx(pos_curr[gl_GlobalInvocationID.x]);

    uint partical_index_in_cell = atomicAdd(grid_partical_count[grid_idx], uint(1));

    partical_index_in_cell = min(cellmaxparticalcount -1, partical_index_in_cell);

    grid_particals[grid_idx * cellmaxparticalcount + partical_index_in_cell].index = gl_GlobalInvocationID.x;
    partical_grid_index[gl_GlobalInvocationID.x] = grid_idx * cellmaxparticalcount + partical_index_in_cell;

}