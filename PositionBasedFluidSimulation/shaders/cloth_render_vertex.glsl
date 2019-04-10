#version 450

in vec3 vVertex;
in vec3 vNorm;

out float color;

uniform mat4 mMV;

void main() {
    gl_Position = mMV * vec4(vVertex, 1.0);


    color = dot(vNorm,vec3(1.0, 0, 0));
    color = abs(color);

}