/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include "stdafx.h"
#include "Input.h"

void InputHandler::HandleMSG(const MSG& msg) {
    switch (msg.message) {
    case WM_KEYDOWN:
        keys_[static_cast<char>(msg.wParam)] = true;
        break;
    case WM_KEYUP:
        keys_[static_cast<char>(msg.wParam)] = false;
        break;
    }
}

bool InputHandler::IsKeyDown(char virtual_key) {
    return keys_[virtual_key];
}
