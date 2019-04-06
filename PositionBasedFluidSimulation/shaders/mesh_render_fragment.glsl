#version 450

in vec3 varyingNorm;

void main(void) {
    float illu = dot(varyingNorm, vec3(1, 2, 3))/sqrt(1+4+9);
    illu = 0.5*(illu + 1.0);
    gl_FragColor = vec4(illu, 0, 0, 0) / sqrt(1+4+9);
}