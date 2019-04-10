#pragma once
#include"predef.h"

#include"camera.h"

namespace boundary {
    extern glm::vec3 bmin;
    extern glm::vec3 bmax;

    void update();

    void toggleWave();

    void render(Camera& camera);

}