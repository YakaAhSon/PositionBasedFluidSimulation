#version 430
// render fluid particals
// vertex shader

in vec2 vVertex;

in vec3 particalPosition;

uniform mat4 mView;
uniform mat4 mProjection;

out vec2 pointCoord;

void main(void)
{
    vec4 vertex_pos = vec4(0.05*vVertex, 0.0, 0.0) + mView * vec4(particalPosition, 1.0);

    gl_Position = mProjection * vertex_pos;

    pointCoord = vVertex;

}