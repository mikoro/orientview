# OrientView Development Conventions

## About

OrientView is a native multi-platform modern C++ program used to analyze orienteering run videos and map routes in real-time.

## Code structure and style

Assume that the target audience is experienced senior developers.
Do not add comments to the code.
Do not add a period to single sentence strings (e.g. log or exception messages).
Avoid forward declarations.
Target modern C++20. Do not use macros if possible and do not use gotos.
Use PascalCase for class and function names and camelCase for variables.
Project is designed to use unity-style build. All code implementation is in .hpp files where the member functions are implemented inline. These .hpp files are then included all in a single main.cpp file.

Following third-party libraries are being used:
- ffmpeg
- fmt
- glm
- imgui
- nlohmann-json
- pugixml
- sdl3
- spdlog

### Formatting example

Here is a format example of the code:

```cpp
#pragma once

#include <string>

enum class ActionType {
    Move
};

class ActionHandler {
private:
    int _count = 0;
    bool _activeFlag = true;

public:
    std::string HandleAction(ActionType type, const std::string& name, int* outCode) {
        if (!_activeFlag) {
            if (outCode) {
                *outCode = -1;
            }

            return "Inactive";
        }

        if (name.empty()) {
            if (outCode) {
                *outCode = -2;
            }

            return "No name";
        }

        if (type == ActionType::Move) {
            _count++;

            if (outCode) {
                *outCode = _count;
            }

            return "Moved";
        } else {
            if (outCode) {
                *outCode = 0;
            }

            return "Unhandled";
        }
    }
};
```
