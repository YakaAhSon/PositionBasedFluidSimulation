#pragma once
#include"predef.h"
class Camera
{
private:
    glm::mat4 m_projection;// projection matrix
    float angle_heading;
    float angle_pitch;
    glm::vec3 v_pos;// center

    glm::vec2 tanfov;
public:
    Camera();

    void setViewFrustum(float fovy, float aspect);

    const glm::mat4 getRotationMatrix()const;
    const glm::mat4 getProjectionMatrix()const;
    const glm::mat4 getViewMatrix()const;
    const glm::mat4 getViewProjectionMatrix()const;

    void moveGlobal(const glm::vec3& step);
    void moveLocal(const glm::vec3& step);

    void moveForward(float d);
    void moveRight(float d);
    void moveUp(float d);

    void rotateRight(float a);
    void rotateUp(float a);

    const glm::vec2& getTanFov()const { return tanfov; }
};