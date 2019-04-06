#pragma once

#ifndef __PBF_SOLID_MODEL_H__

#define __OBF_SOLID_MODEL_H__

#include"predef.h"

#include<vector>

class SolidModel {
public:
    std::vector<glm::vec3>::iterator _partical_begin_;
    std::vector<glm::vec3>::iterator _partical_end_;
    int _partial_count_;

    virtual void init()=0;
};

class SolidWall:public SolidModel {
public:
    virtual void init();
};

#endif