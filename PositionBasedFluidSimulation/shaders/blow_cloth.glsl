#version 450

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

layout(std430, binding = 1)buffer impulse_buffer {
    struct {
        vec3 impulse;
        int padding1;
        vec3 pos;
        int padding2;
    }impulses[];
};

layout(binding = 0, offset = 0) uniform atomic_uint impulse_counter;

uniform vec3 bMin;
uniform vec3 bMax;

void main(void) {
    uint partical_index = gl_GlobalInvocationID.x * 32;

    vec3 pos = particals[partical_index].pos;
    if (any(lessThanEqual(pos, bMin)) || any(greaterThanEqual(pos, bMax)))return;

    uint impulse_idx = atomicCounterIncrement(impulse_counter);

    impulses[impulse_idx].pos = pos;
    impulses[impulse_idx].impulse = pos- particals[partical_index].pos_prev;
}