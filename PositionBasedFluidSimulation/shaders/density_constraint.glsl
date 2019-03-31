#version 450

// compute shader
// compute density constraint
// same result in buffer pos_delta

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};

layout(std430, binding = 1) buffer grid_buffer {
    uint grid[];
};

layout(std430, binding = 2) buffer pos_delta_buffer {
    vec4 pos_delta[];
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
    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz;

    ivec3 grid_v = getGrid(pos);

    ivec3 grid_v_min = grid_v - ivec3(2);
    grid_v_min = max(grid_v_min, ivec3(0));

    ivec3 grid_v_max = grid_v + ivec3(2);
    grid_v_max = min(grid_v_max, ivec3(grid_edge_count - 1));

    int grid_id = getGridIdx(grid_v);

    
    vec4 delta = vec4(0);

    for (uint n = 0; n < 32*1024; n++) {
        if (n != gl_GlobalInvocationID.x) {
            vec3 neighbour = pos_curr[n].xyz;

            vec3 norm = pos - neighbour;
            float l = dot(norm, norm);

            if (l < (0.2*0.2)) {
                l = sqrt(l);
                l = max(l, 0.000001);
                norm = norm / l;
                l = 0.2 - l;
                delta.xyz += norm* l * 0.5;
            }
        }
    }
    pos_delta[gl_GlobalInvocationID.x] = delta;
    return;
    
    for (int x = grid_v_min.x; x <= grid_v_max.x; x++)for (int y = grid_v_min.y; y <= grid_v_max.y; y++)for (int z = grid_v_min.z; z <= grid_v_max.z; z++) {

        int cell_idx = z * grid_edge_count2 + y * grid_edge_count + x;
        uint neighbour_idx = grid[cell_idx];

        if (neighbour_idx != gl_GlobalInvocationID.x && neighbour_idx < 0xffffffff) {

            vec3 neighbour = pos_curr[neighbour_idx].xyz;

            vec3 norm = pos - neighbour;
            float l = dot(norm, norm);

            if (l < (0.5*0.5)) {
                l = sqrt(l);
                l = max(l, 0.000001);
                norm = norm / l;
                l = 0.5 - l;
                norm = norm * l*0.1;
                pos_delta[gl_GlobalInvocationID.x].xyz += norm;

                //pos_curr[gl_GlobalInvocationID.x].xyz += norm;
                //pos_curr[neighbour_idx].xyz -= norm;
            }

        }
    }
}