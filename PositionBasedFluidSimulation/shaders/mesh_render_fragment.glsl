#version 450

in vec3 varyingNorm;

uniform vec3 color;

void main(void) {
    float illu = dot(varyingNorm, vec3(1, 2, 3))/sqrt(1+4+9);
    illu = 0.5*(illu + 0.7);
    gl_FragColor = vec4(color*illu, 0);
}