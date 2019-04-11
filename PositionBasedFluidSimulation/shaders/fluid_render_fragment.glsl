#version 450
// render fluid particals
// fragment shader
// depth map
in  float depth;
uniform int renderPartical;
void main(void) 
{
    gl_FragColor =(renderPartical==1)?vec4(0,0,depth,0): vec4(depth);
}