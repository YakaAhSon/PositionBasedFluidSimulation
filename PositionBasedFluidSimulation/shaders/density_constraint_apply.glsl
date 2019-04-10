#version 450

// compute shader
// apply density constraint

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


void main(void)
{
    particals[gl_GlobalInvocationID.x].pos += particals[gl_GlobalInvocationID.x].delta_p;
}