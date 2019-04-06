#version 450

// compute shader

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec3 pos_curr[];
};

layout(std430, binding = 1) buffer grid_partical_count_buffer {
    uint grid_partical_count[];
};

layout(std430, binding = 2) buffer grid_particals_buffer {
    struct {
        vec3 pos;
    } grid_particals[];
};

layout(std430, binding = 3) buffer partical_grid_buffer {
    uint partical_grid_index[];
};

void main() {
    uint grid_index = partical_grid_index[gl_GlobalInvocationID.x];
    grid_particals[grid_index].pos = pos_curr[gl_GlobalInvocationID.x];
}