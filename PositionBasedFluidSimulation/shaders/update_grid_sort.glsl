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

layout(std430, binding = 1) buffer partical_tmp_buffer {
    struct {
        vec3 pos;
        float lambda;
        vec3 pos_prev;

        uint grid_idx;

        vec3 delta_p;

        uint idx_in_grid;
    }particals_tmp[];
};

layout(std430, binding = 2) buffer grid_buffer {
    struct {
        uint count;
        uint start;
    } grids[];
};

void main(void)
{
    struct {
        vec3 pos;
        float lambda;
        vec3 pos_prev;

        uint grid_idx;

        vec3 delta_p;

        uint idx_in_grid;
    }me = particals[gl_GlobalInvocationID.x];

    uint sorted_idx = grids[me.grid_idx].start + me.idx_in_grid;

    particals_tmp[sorted_idx] = me;
}