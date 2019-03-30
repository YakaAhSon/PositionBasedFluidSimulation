#version 430
// render fluid particals
// fragment shader

in  vec2 pointCoord;

in float color;

void main(void) 
{
    float illu = dot(pointCoord, vec2(1.0f, 1.0f));
    gl_FragColor = vec4(0,0,illu,1)*color;
}