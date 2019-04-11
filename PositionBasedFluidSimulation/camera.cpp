#include "camera.h"

Camera::Camera()
{
    angle_heading = 0;
    angle_pitch = 0;
    v_pos = glm::vec3(0, 0, 0);
    setViewFrustum(glm::pi<float>() / 2, 1.0);
}

void Camera::setViewFrustum(float fovy, float aspect) 
{
    tanfov.y = tanf(fovy / 2.0f);
    tanfov.x = tanfov.y * aspect;
    m_projection = glm::perspective(fovy, aspect, 1.0f, 100.0f);
}

const glm::mat4 Camera::getRotationMatrix() const
{
    glm::mat4 mHeading = glm::rotate(angle_heading, glm::vec3(0, 1, 0));
    glm::mat4 mPitch = glm::rotate(angle_pitch, glm::vec3(1, 0, 0));
    return mHeading * mPitch;
}

const glm::mat4 Camera::getProjectionMatrix() const
{
    return m_projection;
}

const glm::mat4 Camera::getViewMatrix() const
{
    glm::mat4 mR = getRotationMatrix();
    mR = glm::transpose(mR);

    glm::mat4 mT = glm::translate(-v_pos);

    glm::mat4 mView = mR * mT;
    return mView;
}

const glm::mat4 Camera::getViewProjectionMatrix() const
{
    return m_projection * getViewMatrix();
}

void Camera::moveGlobal(const glm::vec3 & step)
{
    v_pos = v_pos + step;
}

void Camera::moveLocal(const glm::vec3& step)
{
    glm::vec4 step_global = getRotationMatrix()*glm::vec4(step, 1.0);
    moveGlobal(glm::vec3(step_global));
}

void Camera::moveForward(float d)
{
    moveLocal(glm::vec3(0, 0, -d));
}

void Camera::moveRight(float d)
{
    moveLocal(glm::vec3(d, 0, 0));
}

void Camera::moveUp(float d)
{
    moveLocal(glm::vec3(0, d, 0));
}

void Camera::rotateRight(float a)
{
    angle_heading -= a;
}

void Camera::rotateUp(float a)
{
    angle_pitch += a;
    angle_pitch = glm::min(angle_pitch, glm::pi<float>()/2);
    angle_pitch = glm::max(angle_pitch, -glm::pi<float>()/2);
}
