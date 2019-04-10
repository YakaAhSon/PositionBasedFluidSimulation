#version 450

// compute shader
// current = current + (current - prev) + gravity*0.5*timestep*timestep

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
    vec3 tmp = particals[gl_GlobalInvocationID.x].pos;

    // g = 9.8
    // 0.5 * g * (1/60) * (1/60) = 0.00136
    vec3 pos= tmp*2 - particals[gl_GlobalInvocationID.x].pos_prev + vec3(0.00000, -0.00136, 0.00000);

    particals[gl_GlobalInvocationID.x].pos_prev = tmp;
    particals[gl_GlobalInvocationID.x].pos = pos;
}