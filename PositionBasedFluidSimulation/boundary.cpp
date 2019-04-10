#include"boundary.h"

glm::vec3 boundary::bmin = glm::vec3(-11.5, -5.5, -5.5);
glm::vec3 boundary::bmax = glm::vec3(11.5, 5.5, 5.5);

void boundary::update()
{

    static float timer = 0;

    bmin.x = -9.0 - glm::cos(timer);
    timer += 0.05;
}
