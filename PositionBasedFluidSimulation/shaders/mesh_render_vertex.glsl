#version 450

in vec3 vVertex;
in vec3 vNorm;

uniform mat4 mViewProjection;

out vec3 varyingNorm;

void main(void) {
    varyingNorm = vNorm;
    gl_Position = mViewProjection* vec4(vVertex+vec3(0,-2,0), 1.0);
}