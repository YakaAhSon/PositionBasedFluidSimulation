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
    } grid_particals[];
};

layout(std430, binding = 3) buffer pos_delta_buffer {
    vec4 pos_delta[];
};

layout(std430, binding = 4) buffer partical_grid_buffer {
    uint partical_grid_index[];
};

uniform float cellsize;

uniform int grid_edge_count;

uniform int cellmaxparticalcount;

uniform float kernel_radius;


float POLY6(float r2) {

    float kernel_r2 = 0.1681;

    // 315/(64*pi*h^9)
    return (kernel_r2 - r2)*(kernel_r2 - r2)*(kernel_r2 - r2)*4785.48397271887;
}
float POLY6_gradient(float r2) {
    float kernel_r2 = 0.1681;


    // 945/(32*pi*h^9)
    return -(kernel_r2 - r2)*(kernel_r2 - r2)*28712.641146192094;
}

float scorr(float r2) {
    float tmp = POLY6(r2) / POLY6(0.001225);

    return -0.1*tmp*tmp*tmp*tmp;
}

const float rho0 = 1000.0;

void main(void)
{

    int grid_edge_count2 = grid_edge_count * grid_edge_count;

    vec4 pos_full = pos_curr[gl_GlobalInvocationID.x];

    vec3 pos = pos_full.xyz;

    vec3 pos_min = pos - vec3(kernel_radius*0.8);
    vec3 pos_max = pos + vec3(kernel_radius*0.8);

    ivec3 grid_v = ivec3((pos + vec3(6, 6, 6)) / cellsize);

    ivec3 grid_v_min = ivec3((pos_min + vec3(6, 6, 6)) / cellsize);
    ivec3 grid_v_max = ivec3((pos_max + vec3(6, 6, 6)) / cellsize);
    vec3 deltaP = vec3(0);

    float kernel_radius2 = kernel_radius * kernel_radius;

    uint my_idx = partical_grid_index[gl_GlobalInvocationID.x];
    float lambda_i = grid_particals[my_idx].lambda;

    // for each neighbiyr cell

    for (int x = grid_v_min.x; x <= grid_v_max.x; x++)for (int y = grid_v_min.y; y <= grid_v_max.y; y++)for (int z = grid_v_min.z; z <= grid_v_max.z; z++) {

        uint cellidx = z * grid_edge_count2 + y * grid_edge_count + x;

        uint partical_count = grid_partical_count[cellidx];

        // for each partical in the neightbour cell
        for (uint i = 0; i < partical_count; i++) {

            struct {
                vec3 pos;
                float lambda;
            }neighbour = grid_particals[cellidx*cellmaxparticalcount + i];



            vec3 norm = pos - neighbour.pos;

            float r2 = dot(norm, norm);

            if (r2 < kernel_radius2&& r2>0.0000001) {
                float lambda_j = neighbour.lambda;
                float g = POLY6_gradient(r2);
                norm = norm * g;
                deltaP += norm * (lambda_i + lambda_j + scorr(r2)) / rho0;

            }



        }


    }
    float l = length(deltaP);
    float dl = min(l, 0.3);
    deltaP = deltaP / l * dl;
    pos_delta[gl_GlobalInvocationID.x] = vec4(deltaP, 0);
}