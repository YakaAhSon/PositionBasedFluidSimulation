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

layout(std430, binding = 2) buffer grid_particals_buffer {
    struct {
        vec3 pos;
        uint index;
    }grid_particals[];
};

uniform float cellsize;

uniform int grid_edge_count;

uniform int cellmaxparticalcount;

uniform float kernel_radius;

float POLY6(float r) {

    float kernel_r2 = kernel_radius* kernel_radius;
    float kernel_r9 = kernel_r2 * kernel_r2 * kernel_r2 * kernel_r2 * kernel_radius;

    // 315/(64*pi*h^9)
    return (kernel_r2 - r*r)*(kernel_r2 - r*r)*(kernel_r2 - r*r)*1.566681 /kernel_r9;
}
float POLY6_gradient(float r) {
    float kernel_r2 = kernel_radius * kernel_radius;

    float kernel_r9 = kernel_r2 * kernel_r2 * kernel_r2 * kernel_r2 * kernel_radius;

    // 945/(32*pi*h^9)
    return -r * (kernel_r2 - r * r)*(kernel_r2 - r * r)*9.40008826 / kernel_r9;
}

const float rho0 = 1000.0;
const float partical_weight = 8.0;// rho0*0.1*0.1*0.1

void main(void)
{
    int grid_edge_count2 = grid_edge_count * grid_edge_count;

    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz;

    ivec3 grid_v = ivec3((pos + vec3(6, 6, 6)) / cellsize);

    ivec3 grid_v_min = grid_v - ivec3(1);
    ivec3 grid_v_max = grid_v + ivec3(1);

    float rho = partical_weight*POLY6(0);

    vec3 gradient_i = vec3(0);
    float gradient_j_2 = 0;

    // for each neighbour cell
    for (int x = grid_v_min.x; x <= grid_v_max.x; x++)for (int y = grid_v_min.y; y <= grid_v_max.y; y++)for (int z = grid_v_min.z; z <= grid_v_max.z; z++) {

        uint cellidx = z * grid_edge_count2 + y * grid_edge_count + x;

        uint partical_count = grid_partical_count[cellidx];

        // for each partical in the neightbour cell
        for (uint i = 0; i < partical_count; i++) {

            uint neighbour_idx = grid_particals[cellidx*cellmaxparticalcount + i].index;
            if (neighbour_idx != gl_GlobalInvocationID.x) {

                vec3 norm = grid_particals[cellidx*cellmaxparticalcount + i].pos - pos;

                float r = length(norm);

                if (r < kernel_radius) {

                    rho += partical_weight * POLY6(r);

                    r = max(r, 0.00001);
                    norm = norm / r;// normalize

                    float g = POLY6_gradient(r);
                    norm = norm * g;

                    gradient_i += norm;
                    gradient_j_2 += g * g;

                }


            }
        }


    }

    float C = rho / rho0 - 1;
    
    // lambda = - C/(gradient_j_2 + gradient_i^2)
    pos_curr[gl_GlobalInvocationID.x].w = -C / (gradient_j_2 + dot(gradient_i, gradient_i) + 3.0*rho0*rho0)*rho0*rho0;
}
