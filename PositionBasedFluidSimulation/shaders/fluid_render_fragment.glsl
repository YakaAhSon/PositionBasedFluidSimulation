#version 450
// render fluid particals
// fragment shader
// depth map
in  float depth;

void main(void) 
{
    gl_FragColor = vec4(depth);
}