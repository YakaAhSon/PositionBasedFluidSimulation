#version 450

in vec2 TexCoords;

uniform sampler2DRect depthTexture;

uniform float width_height;

vec3 uvToEye(vec2 vtex, float depth){
	vec2 v = vtex*2-vec2(1);
	float y = v.y*depth;
	float x = v.y*width_height;
	return vec3(x,y,depth);
}

void main()
{
    ivec2 texCoordI = ivec2(TexCoords*1024.0);
    float depth = texture(depthTexture, texCoordI).x;

	vec3 normal;
	
    gl_FragColor = vec4(col, 1.0);
} 