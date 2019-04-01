#version 450

// compute shader
// calculate lambda
// save lambda in pos_curr[].w

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

uniform int cellmaxparticalcount;

uniform float kernel_radius;

float POLY6(float r) {
    float kernel_r2 = kernel_radius * kernel_radius;
    float kernel_r9 = kernel_r2 * kernel_r2 * kernel_r2 * kernel_r2 * kernel_radius;
    // 315/(64*pi*h^9)
    return (kernel_r2 - r * r)*(kernel_r2 - r * r)*(kernel_r2 - r * r)*1.566681471 / kernel_r9;
}
float POLY6_gradient(float r) {
    float kernel_r2 = kernel_radius * kernel_radius;

    float kernel_r9 = kernel_r2 * kernel_r2 * kernel_r2 * kernel_r2 * kernel_radius;

    // 945/(32*pi*h^9)
    return -r * (kernel_r2 - r * r)*(kernel_r2 - r * r)*9.40008826 / kernel_r9;
}

float scorr(vec3 p1, vec3 p2) {
    float r = length(p1 - p2);
    float tmp = POLY6(r) / POLY6(0.1*kernel_radius);

    return -0.1*tmp*tmp*tmp*tmp;
}

const float rho0 = 1000.0;

void main(void)
{

    int grid_edge_count2 = grid_edge_count * grid_edge_count;

    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz;

    ivec3 grid_v = ivec3((pos + vec3(6, 6, 6)) / cellsize);

    ivec3 grid_v_min = grid_v - ivec3(1);
    ivec3 grid_v_max = grid_v + ivec3(1);

    vec3 deltaP = vec3(0);

    // for each neighbiyr cell
    for (int x = grid_v_min.x; x <= grid_v_max.x; x++)for (int y = grid_v_min.y; y <= grid_v_max.y; y++)for (int z = grid_v_min.z; z <= grid_v_max.z; z++) {

        uint cellidx = z * grid_edge_count2 + y * grid_edge_count + x;

        uint partical_count = grid_partical_count[cellidx];

        // for each partical in the neightbour cell
        for (uint i = 0; i < partical_count; i++) {

            uint neighbour_idx = grid_partical_index[cellidx*cellmaxparticalcount + i];

            if (neighbour_idx != gl_GlobalInvocationID.x) {

                vec3 norm = pos - pos_curr[neighbour_idx].xyz;

                float r = length(norm);

                if (r < kernel_radius) {

                    float g = POLY6_gradient(r);
                    norm = norm / r * g;
                    deltaP += norm * (pos_curr[gl_GlobalInvocationID.x].w + pos_curr[neighbour_idx].w + scorr(pos_curr[gl_GlobalInvocationID.x].xyz, pos_curr[neighbour_idx].xyz)) / rho0;

                }


            }
        }


    }

    pos_delta[gl_GlobalInvocationID.x] = vec4(deltaP, 0);
}