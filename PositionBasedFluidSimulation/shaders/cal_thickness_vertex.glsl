#version 450
// render fluid particals
// vertex shader

layout(std430, binding = 0) buffer partical_buffer {
    struct {
        vec3 pos;
        float lambda;
        vec3 pos_prev;

        uint grid_idx;

        vec3 delta_p;

        uint idx_in_grid;
    }particals[];
};

in vec2 vVertex;


uniform mat4 mView;
uniform mat4 mProjection;

void main(void)
{

    vec3 particalPosition = particals[gl_InstanceID].pos;

    vec4 pos_view = mView * vec4(particalPosition, 1.0);


    vec4 vertex_pos = vec4((0.2)*vVertex, 0.0, 0.0) + pos_view;

    gl_Position = mProjection * vertex_pos;

    float color = dot(vVertex, vec2(1, 1))*0.5 + 0.5;

}