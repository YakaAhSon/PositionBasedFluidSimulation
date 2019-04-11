#version 450

layout(std430, binding = 0)buffer normal_buffer {
    vec4 normals[];
};


in vec2 TexCoords;

uniform sampler2DRect thicknessTexture;

uniform mat4 mProjection;

uniform vec2 tanfov;

void main()
{

	ivec2 t = ivec2(TexCoords*1024.0);

	float thickness = texture(thicknessTexture, ivec2(TexCoords*1024.0)).x;


	if(thickness>0){
		gl_FragDepth = 0.0;
	}
	else{
		gl_FragDepth = 20.0;
	}


    //float col = texture(screenTexture, ivec2(TexCoords*1024.0)).x;
	
    //gl_FragColor = vec4(col,col,col, 1.0);
	gl_FragColor = normals[t.x*1024+t.y];
	gl_FragColor.a = thickness*500+0.5;
} 