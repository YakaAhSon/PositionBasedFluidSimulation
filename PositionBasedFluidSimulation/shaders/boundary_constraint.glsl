#version 450

// compute shader
// current = current + (current - prev) + gravity*0.5*timestep*timestep

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;


layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};


uniform vec3 bmin;
uniform vec3 bmax;

void main(void)
{
    vec4 pos = pos_curr[gl_GlobalInvocationID.x];

    pos = max(pos, vec4(bmin,0.0));

    pos = min(pos, vec4(bmax, 0.0));
    pos_curr[gl_GlobalInvocationID.x] = pos;
}