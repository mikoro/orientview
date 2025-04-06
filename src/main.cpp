#include "app.hpp"

#include <spdlog/spdlog.h>

#include <exception>

int main(int argc, char* argv[]) {
    try {
        App app;
        return app.Run();
    } catch (const std::exception& e) {
        spdlog::critical("Unhandled exception: {}", e.what());
        return -1;
    } catch (...) {
        spdlog::critical("Unhandled unknown exception");
        return -1;
    }
}
