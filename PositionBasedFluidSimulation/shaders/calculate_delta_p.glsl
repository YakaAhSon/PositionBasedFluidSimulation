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
    } grid_particals[];
};

layout(std430, binding = 3) buffer pos_delta_buffer {
    vec4 pos_delta[];
};

uniform float cellsize;

uniform int grid_edge_count;

uniform int cellmaxparticalcount;

uniform float kernel_radius;


float POLY6(float r) {

    float kernel_r2 = 0.1681;

    // 315/(64*pi*h^9)
    return (kernel_r2 - r * r)*(kernel_r2 - r * r)*(kernel_r2 - r * r)*4785.48397271887;
}
float POLY6_gradient(float r) {
    float kernel_r2 = 0.1681;


    // 945/(32*pi*h^9)
    return -r * (kernel_r2 - r * r)*(kernel_r2 - r * r)*28712.641146192094;
}

float scorr(float r) {
    float tmp = POLY6(r) / POLY6(0.1*kernel_radius);

    return -0.1*tmp*tmp*tmp*tmp;
}

const float rho0 = 1000.0;

void main(void)
{

    int grid_edge_count2 = grid_edge_count * grid_edge_count;
    
    vec4 pos_full = pos_curr[gl_GlobalInvocationID.x];

    float lambda_i = pos_full.w;

    vec3 pos = pos_full.xyz;

    vec3 pos_min = pos - vec3(0.41);
    vec3 pos_max = pos + vec3(0.41);

    ivec3 grid_v = ivec3((pos + vec3(6, 6, 6)) / cellsize);

    ivec3 grid_v_min = ivec3((pos_min + vec3(6, 6, 6)) / cellsize);
    ivec3 grid_v_max = ivec3((pos_max + vec3(6, 6, 6)) / cellsize);
    vec3 deltaP = vec3(0);

    // for each neighbiyr cell
    for (int i = 0; i < 27;i++) {

        uint x = i % 3+grid_v_min.x;
        uint y = (i / 3) % 3+grid_v_min.y;
        uint z = (i / 9) % 3+grid_v_min.z;

        uint cellidx = z * grid_edge_count2 + y * grid_edge_count + x;

        uint partical_count = grid_partical_count[cellidx];

        // for each partical in the neightbour cell
        for (uint i = 0; i < partical_count; i++) {

            uint neighbour_idx = grid_particals[cellidx*cellmaxparticalcount + i].index;
            vec3 neighbour_pos = grid_particals[cellidx*cellmaxparticalcount + i].pos;


            if (neighbour_idx != gl_GlobalInvocationID.x) {

                vec3 norm = pos - neighbour_pos;

                float r = length(norm);

                if (r < kernel_radius) {

                    float lambda_j = pos_curr[neighbour_idx].w;
                    float g = POLY6_gradient(r);
                    norm = norm / r * g;
                    deltaP += norm * (lambda_i+lambda_j + scorr(r)) / rho0;

                }


            }
        }


    }
    float l = length(deltaP);
    float dl = min(l, 0.3);
    deltaP = deltaP / l * dl;
    pos_delta[gl_GlobalInvocationID.x] = vec4(deltaP, 0);
}