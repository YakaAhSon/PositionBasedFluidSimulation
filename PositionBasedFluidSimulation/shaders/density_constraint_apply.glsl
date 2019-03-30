#version 430

// compute shader
// apply density constraint

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec3 pos_curr[];
};

layout(std430, binding = 2) buffer pos_delta_buffer {
    vec3 pos_delta[];
};

void main(void)
{
    pos_curr[gl_GlobalInvocationID.x] += pos_delta[gl_GlobalInvocationID.x];
}