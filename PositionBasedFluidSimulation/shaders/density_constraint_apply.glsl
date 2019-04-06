#version 450

// compute shader
// apply density constraint

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};

layout(std430, binding = 2) buffer pos_delta_buffer {
    vec4 pos_delta[];
};

void main(void)
{
    pos_curr[gl_GlobalInvocationID.x].xyz +=  pos_delta[gl_GlobalInvocationID.x].xyz;
}