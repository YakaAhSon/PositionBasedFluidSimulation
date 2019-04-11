#version 450
layout (location = 0) in vec2 aPos;
//layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = 0.5*(aPos+vec2(1,1));
    gl_Position = vec4(aPos.x, aPos.y, 0.999, 1.0); 
}  