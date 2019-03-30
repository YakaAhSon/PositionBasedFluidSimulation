#version 430

// compute shader
// solve density constraint

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};

layout(std430, binding = 1) buffer grid_buffer {
    int grid[];
};

uniform float grid_size;
uniform int grid_edge_count;
uniform int grid_edge_count2;


ivec3 edgeRestrict(ivec3 iv) {
    iv = max(iv, ivec3(0, 0, 0));
    return min(iv, ivec3(grid_edge_count - 1));
}

ivec3 getGrid(vec3 v) {
    ivec3 iv = ivec3((v + vec3(5, 5, 5)) / grid_size);

    return edgeRestrict(iv);
}

int getGridIdx(vec3 v) {
    ivec3 iv = getGrid(v);
    return iv.z*grid_edge_count2 + iv.y*grid_edge_count + iv.x;
}

void main(void)
{
    ivec3 grid_v = getGrid(pos_curr[gl_GlobalInvocationID.x].xyz);

    ivec3 grid_v_min = grid_v - ivec3(2, 2, 2);
    grid_v_min = max(grid_v_min, ivec3(0));

    ivec3 grid_v_max = grid_v + ivec3(2, 2, 2);
    grid_v_min = min(grid_v_min, ivec3(grid_edge_count - 1));

    int grid_id = getGridIdx(grid_v);

    for (int x = grid_v_min.x; x < grid_v_max.x; x++) {
        for (int y = grid_v_min.y; y < grid_v_max.y; y++) {
            for (int z = grid_v_min.z; z < grid_v_max.z; z++) {
                int cell_idx = iv.z*grid_edge_count2 + iv.y*grid_edge_count + iv.x;
            }
        }
    }
}