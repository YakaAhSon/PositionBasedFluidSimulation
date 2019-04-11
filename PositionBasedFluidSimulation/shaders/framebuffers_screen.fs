#version 450

layout(std430, binding = 0)buffer normal_buffer {
    vec4 normals[];
};


in vec2 TexCoords;

uniform sampler2DRect depthTexture;

uniform mat4 mProjection;

uniform vec2 tanfov;

void main()
{

	ivec2 t = ivec2(TexCoords*1024.0);
	float depth = normals[t.x*1024+t.y].w;

	depth = texture(depthTexture, ivec2(TexCoords*1024.0)).x*36;

    vec4 me =  vec4(0,0,-depth,1.0);

	vec4 me_p = mProjection*me;

	if(0<depth&& depth<36){
		
		gl_FragDepth = me_p.z/me_p.w-2.0;
	}
	else{
		gl_FragDepth = 2.0;
	}


    //float col = texture(screenTexture, ivec2(TexCoords*1024.0)).x;
	
    //gl_FragColor = vec4(col,col,col, 1.0);
	gl_FragColor = normals[t.x*1024+t.y];
	gl_FragColor.a = 0.7;
} 