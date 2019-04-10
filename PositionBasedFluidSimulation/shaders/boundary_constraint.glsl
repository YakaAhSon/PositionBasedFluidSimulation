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


uniform vec3 bmin;
uniform vec3 bmax;

void main(void)
{
    vec3 pos = particals[gl_GlobalInvocationID.x].pos;

    pos = max(pos, bmin);

    pos = min(pos, bmax);

    particals[gl_GlobalInvocationID.x].pos = pos;
}