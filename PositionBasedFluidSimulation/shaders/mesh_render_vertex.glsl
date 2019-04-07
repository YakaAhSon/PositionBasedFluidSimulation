#version 450

in vec3 vVertex;
in vec3 vNorm;

uniform mat4 mMVP;
uniform mat3 mModelRot;

out vec3 varyingNorm;

void main(void) {
    varyingNorm = mModelRot*vNorm;
    gl_Position = mMVP* vec4(vVertex, 1.0);
}