/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once

class InputHandler {
 public:
    void HandleMSG(const MSG& msg);
    bool IsKeyDown(char virtual_key);
 private:
    bool keys_[256]{};
};
