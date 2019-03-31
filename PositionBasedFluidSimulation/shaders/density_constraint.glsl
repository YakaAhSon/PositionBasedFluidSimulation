#version 450

// compute shader
// compute density constraint
// same result in buffer pos_delta

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1)in;

layout(std430, binding = 0) buffer pos_curr_buffer {
    vec4 pos_curr[];
};

layout(std430, binding = 1) buffer grid_partical_count_buffer {
    uint grid_partical_count[];
};

layout(std430, binding = 2) buffer grid_partical_index_buffer {
    uint grid_partical_index[];
};

layout(std430, binding = 3) buffer pos_delta_buffer {
    vec4 pos_delta[];
};


uniform float cellsize;

uniform int grid_edge_count;
uniform int grid_edge_count2;

uniform int cellmaxparticalcount;

void main(void)
{
    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz;

    ivec3 grid_v = ivec3((pos + vec3(6, 6, 6)) / cellsize);

    ivec3 grid_v_min = grid_v - ivec3(1);
    ivec3 grid_v_max = grid_v + ivec3(1);

    grid_v_max = min(grid_v_max, ivec3(grid_edge_count));

    pos_delta[gl_GlobalInvocationID.x] = vec4(0);
    for (int x = grid_v_min.x; x <= grid_v_max.x; x++)for (int y = grid_v_min.y; y <= grid_v_max.y; y++)for (int z = grid_v_min.z; z <= grid_v_max.z; z++) {

        uint cellidx = z * grid_edge_count2 + y * grid_edge_count + x;

        uint partical_count = grid_partical_count[cellidx];

        for (uint i = 0; i < partical_count; i++) {

            uint neighbour_idx = grid_partical_index[cellidx*cellmaxparticalcount + i];

            if (neighbour_idx != gl_GlobalInvocationID.x) {

                vec3 neighbour = pos_curr[neighbour_idx].xyz;

                vec3 norm = pos - neighbour;

                float l = dot(norm, norm);

                l = sqrt(l);
                l = max(l, 0.000001);
                norm = norm / l;
                l = 0.15 - l;

                norm = norm * l*0.5*float(int(l > 0));
                pos_delta[gl_GlobalInvocationID.x].xyz += norm;

            }
        }

        
    }
}