#version 450

in vec2 TexCoords;

uniform sampler2DRect screenTexture;

void main()
{
    float col = texture(screenTexture, ivec2(TexCoords*1024.0)).x;
    gl_FragColor = vec4(col,col,col, 1.0);
} 