#version 430
// render fluid particals
// vertex shader

in vec2 vVertex;

in vec4 particalPosition;

uniform mat4 mView;
uniform mat4 mProjection;

out vec2 pointCoord;

uniform float partical_radius;

out float color;
void main(void)
{
    //color = particalPosition.w;
    color = 1.0f;
    vec4 vertex_pos = vec4(0.05*vVertex, 0.0, 0.0) + mView * vec4(particalPosition.xyz, 1.0);

    gl_Position = mProjection * vertex_pos;

    pointCoord = vVertex;

}