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
        float lambda;
    }grid_particals[];
};

layout(std430, binding = 3) buffer partical_grid_buffer {
    uint partical_grid_index[];
};

uniform float cellsize;

uniform int grid_edge_count;

uniform int cellmaxparticalcount;

uniform float kernel_radius;

float POLY6(float r2) {

    float kernel_r2 = 0.1681;
    float kernel_r9 = 0.0003273819343939608;

    // 315/(64*pi*h^9)
    return (kernel_r2 - r2)*(kernel_r2 - r2)*(kernel_r2 - r2)*4785.48397271887;
}
float POLY6_gradient(float r) {
    float kernel_r2 = 0.1681;

    float kernel_r9 = 0.0003273819343939608;

    // 945/(32*pi*h^9)
    return -r * (kernel_r2 - r * r)*(kernel_r2 - r * r)*28712.641146192094;
}

const float rho0 = 800.0;
const float partical_weight = 8.0;// rho0*0.1*0.1*0.1

void main(void)
{
    int grid_edge_count2 = grid_edge_count * grid_edge_count * 2;

    vec3 pos = pos_curr[gl_GlobalInvocationID.x].xyz;

    vec3 pos_min = pos - vec3(kernel_radius*0.9);
    vec3 pos_max = pos + vec3(kernel_radius*0.9);

    ivec3 grid_v_min = ivec3((pos_min + vec3(12, 6, 6)) / cellsize);
    ivec3 grid_v_max = ivec3((pos_max + vec3(12, 6, 6)) / cellsize);

    float rho = partical_weight * POLY6(0);

    vec3 gradient_i = vec3(0);
    float gradient_j_2 = 0;

    float kernel_radius2 = kernel_radius * kernel_radius;


    // for each neighbour cell
    for (int x = grid_v_min.x; x <= grid_v_max.x; x++)for (int y = grid_v_min.y; y <= grid_v_max.y; y++)for (int z = grid_v_min.z; z <= grid_v_max.z; z++) {

        uint cellidx = z * grid_edge_count2 + y * grid_edge_count*2 + x;

        uint partical_count = grid_partical_count[cellidx];

        // for each partical in the neightbour cell
        for (uint i = 0; i < partical_count; i++) {

            struct {
                vec3 pos;
                float lambda;
            }neighbour = grid_particals[cellidx*cellmaxparticalcount + i];
            //uint neighbour_idx = 0;

            vec3 norm = neighbour.pos - pos;

            float r2 = dot(norm, norm);


            if (r2 < kernel_radius2&& r2>0.00001) {

                float r = sqrt(r2);

                rho += partical_weight * POLY6(r2);

                r = max(r, 0.001);
                norm = norm / r;// normalize

                float g = POLY6_gradient(r);
                norm = norm * g;

                gradient_i += norm;
                gradient_j_2 += g * g;

            }



        }


    }

    float C = rho / rho0 - 1;


    uint my_idx = partical_grid_index[gl_GlobalInvocationID.x];

    // lambda = - C/(gradient_j_2 + gradient_i^2)

    grid_particals[my_idx].lambda = -C / (gradient_j_2 + dot(gradient_i, gradient_i) + 3.0*rho0*rho0)*rho0*rho0;
}
