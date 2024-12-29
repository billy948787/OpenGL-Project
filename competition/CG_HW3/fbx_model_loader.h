#pragma once

#include "headers.h"
#include "scene.h"

class FbxModelLoader
{
public:
    FbxModelLoader();
    ~FbxModelLoader();

    virtual bool loadFbx(const std::string &filePath, Scene *scene) = 0;
};