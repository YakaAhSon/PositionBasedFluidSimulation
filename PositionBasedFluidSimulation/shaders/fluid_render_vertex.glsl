#version 430
// render fluid particals
// vertex shader

in vec3 vVertex;
uniform mat4 mVP;// model view projection matrix

void main(void)
{
    gl_Position = mVP * vec4(vVertex, 1.0f);
}