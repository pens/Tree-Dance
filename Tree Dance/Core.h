/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <vector>
#include "Timer.h"
#include "Camera.h"
#include "Rendering.h"
#include "Input.h"

class Core {
 public:
    Core();
    void Init(HWND hwnd);
    void Deinit();
    float Run();
    void HandleMSG(const MSG& msg);

 private:
    Timer timer_{};
    Camera camera_{};
    Renderer renderer_{};
    InputHandler input_{};
    std::vector<Light> lights{};

    Model tree_{};
    TransformSet tree_trans_{};
    Model tree_large_{};
    TransformSet tree_large_trans_{};
    Model terrain_{};
    TransformSet terrain_trans_{};
    Model barrier_{};
    TransformSet barrier_trans_{};

    double time{};
};
