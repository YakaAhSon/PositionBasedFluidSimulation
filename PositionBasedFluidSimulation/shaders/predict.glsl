#version 430

// compute shader
// current = current + (current - prev) + gravity*0.5*timestep*timestep

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1)in;


layout(std430, binding = 0) buffer pos_curr_buffer {
    vec3 pos_curr[];
};

layout(std430, binding = 1) buffer pos_prev_buffer {
    vec3 pos_prev[];
};


void main(void)
{
    vec3 tmp = pos_curr[gl_GlobalInvocationID.x];

    // g = 9.8
    // 0.5 * g * (1/60) * (1/60) = 0.00136
    pos_curr[gl_GlobalInvocationID.x] = pos_curr[gl_GlobalInvocationID.x] * 2 - pos_prev[gl_GlobalInvocationID.x] + vec3(0, -0.00136, 0);

    pos_prev[gl_GlobalInvocationID.x] = tmp;

}