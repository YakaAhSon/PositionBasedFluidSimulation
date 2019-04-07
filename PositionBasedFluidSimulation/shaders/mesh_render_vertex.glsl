#version 450

in vec3 vVertex;
in vec3 vNorm;

uniform mat4 mMVP;

out vec3 varyingNorm;

void main(void) {
    varyingNorm = vNorm;
    gl_Position = mMVP* vec4(vVertex, 1.0);
}