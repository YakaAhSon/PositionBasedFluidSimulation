#version 450

// compute shader
// current = current + (current - prev) + gravity*0.5*timestep*timestep

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;


layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};

layout(std430, binding = 1) buffer pos_prev_buffer {
    vec4 pos_prev[];
};

void main(void)
{
    vec4 tmp = pos_curr[gl_GlobalInvocationID.x];

    // g = 9.8
    // 0.5 * g * (1/60) * (1/60) = 0.00136
    vec4 pos= tmp*2 - pos_prev[gl_GlobalInvocationID.x] + vec4(0.00000, -0.00136, 0.00000,0.000);

    pos_prev[gl_GlobalInvocationID.x] = tmp;

    pos_curr[gl_GlobalInvocationID.x] = pos;
}