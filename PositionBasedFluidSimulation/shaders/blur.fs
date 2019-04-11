#version 450

// https://github.com/JAGJ10/PositionBasedFluids/blob/master/PositionBasedFluids/blur.frag
in vec2 TexCoords;

uniform sampler2DRect depthTexture;

uniform vec2 blurDir;
uniform float filterRadius;

const float blurScale = 10.0f;
const float blurDepthFalloff = 65.0f;

void main()
{
    float depth = texture(depthTexture, ivec2(TexCoords*1024.0)).x;

	float sum = 0.0f;
	float wsum = 0.0f;

	for (float x = -5; x <= 5; x += 1.0f) {
		float s = texture(depthTexture, ivec2((TexCoords + vec2(0,x))*1024.0)).x;

		float r = x * blurScale;
		float w = exp(-r*r);
		
		float r2 = (s - depth) * blurDepthFalloff;
		float g = exp(-r2*r2);
		
		sum += s;
		wsum += 1;
	}

	if (wsum > 0.0f) {
		sum /= wsum;
	}

	float sum0= sum;

	sum=0;
	wsum=0;

	for (float x = -5; x <= 5; x += 1.0f) {
		float s = texture(depthTexture, ivec2((TexCoords + vec2(x,0))*1024.0)).x;

		float r = x * blurScale;
		float w = exp(-r*r);
		
		float r2 = (s - depth) * blurDepthFalloff;
		float g = exp(-r2*r2);
		
		sum += s;
		wsum += 1;
	}
	if (wsum > 0.0f) {
		sum /= wsum;
	}

    gl_FragColor = vec4((sum+sum0)/2);
} 