#version 450
// vertex shader
// voxelize the mesh surface
// screen_width = screen_height = 128


in vec3 vVertex;
in vec3 vNormal;

out vec3 surfacePos;
out vec3 varyingVVertex;
out vec3 varyingVNormal;

uniform float erodeDistance;

uniform float voxelSize;

uniform vec3 bBoxMin;

void main(void) {
    surfacePos = vVertex;
    varyingVVertex = ((vVertex - vNormal * erodeDistance)-bBoxMin) /voxelSize;
    varyingVNormal = vNormal;

    vec3 norm = vNormal;
    vec3 pos = vVertex;
    // rotate to let the triangle face camera
    // step 1. rotate around x axis
    float l = dot(norm.yz, norm.yz);
    if (l > 0.00001) {
        l = sqrt(l);
        float a = norm.y > 0 ? acos(norm.z / l) : -acos(norm.z / l);
        mat2 mRot;
        mRot[0] = vec2(cos(a), -sin(a));
        mRot[1] = vec2(sin(a), cos(a));
        norm.zy = mRot * norm.zy;
        pos.zy = mRot * pos.zy;
    }
    // step 2. rotate around y axis
    l = dot(norm.xz, norm.xz);
    if (l > 0.00001) {
        l = sqrt(l);
        float a = norm.x > 0 ? acos(norm.z / l) : -acos(norm.z / l);
        mat2 mRot;
        mRot[0] = vec2(cos(a), -sin(a));
        mRot[1] = vec2(sin(a), cos(a));
        pos.zx = mRot * pos.zx;
    }
    gl_Position = vec4(pos.x / 3.0, pos.y / 3.0, 0.0, 1.0);
}