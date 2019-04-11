#version 450
// https://github.com/JAGJ10/PositionBasedFluids/blob/master/PositionBasedFluids/blur.frag
in vec2 TexCoords;

uniform sampler2DRect depthTexture;

uniform vec2 blurDir;
uniform float filterRadius;

const float blurScale = 0.1f;
const float blurDepthFalloff = 65.0f;

void main()
{
    float depth = texture(depthTexture, ivec2(TexCoords*1024.0)).x;

	float sum = 0.0f;
	float wsum = 0.0f;

	for (float x = -filterRadius; x <= filterRadius; x += 1.0f) {
		float s = texture(depthTexture, ivec2((TexCoords + x * blurDir)*1024.0)).x;

		float r = x * blurScale;
		float w = exp(-r*r);
		
		float r2 = (s - depth) * blurDepthFalloff;
		float g = exp(-r2*r2);
		
		sum += s * w * g;
		wsum += w * g;
	}

	if (wsum > 0.0f) {
		sum /= wsum;
	}

	vec3 col = vec3(sum);
	
    gl_FragColor = vec4(col, 1.0);
} 