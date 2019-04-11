#version 450

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1)in;

layout(std430, binding = 0)buffer normal_buffer {
    vec4 normals[];
};

uniform sampler2DRect depthTexture;

// tangent fov/2
uniform vec2 tanfov;

vec3 uvToEye(ivec2 vTex){
	
    float depth = texture(depthTexture, vTex).x*36;

    vec2 v = vec2(vTex / 512.0-vec2(1))*tanfov*depth;

    return vec3(v,depth);
}

void main()
{
    ivec2 tmp = ivec2(gl_GlobalInvocationID.xy);
    vec3 pleft = uvToEye(tmp - ivec2(1, 0));
    vec3 pright = uvToEye(tmp + ivec2(1, 0));

    vec3 pup = uvToEye(tmp - ivec2(0, 1));
    vec3 pdown = uvToEye(tmp + ivec2(0, 1));

    vec3 vx = pright - pleft;
    vec3 vy = pup - pdown;

    vec3 normal = normalize(cross(vx, vy));
	
    float color = dot(normal, vec3(1, 1, -1))/2.0;
	
    normals[tmp.x * 1024 + tmp.y] = vec4(0,0,color,pleft.z);
    //normals[tmp.x * 1024 + tmp.y] = vec4(vx*100, 0);
} 