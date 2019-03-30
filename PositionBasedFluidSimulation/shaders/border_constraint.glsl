#version 430

// compute shader
// constraint the current position to the box (-5,5)

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec3 pos_curr[];
};

void main(void)
{
    pos_curr[gl_GlobalInvocationID.x] = max(pos_curr[gl_GlobalInvocationID.x], vec3(-5.0, -5.0, -5.0));
    pos_curr[gl_GlobalInvocationID.x] = min(pos_curr[gl_GlobalInvocationID.x], vec3(5.0, 5.0, 5.0));
}