#include "sd360/App.hpp"
#include "DesktopPlatform.hpp"

#include <iostream>
#include <memory>

int main(int argc, char** argv) {
    const std::filesystem::path config = argc > 1 ? argv[1] : "config/seriesdash.ini";
    sd360::App app(std::make_unique<sd360::DesktopPlatform>());
    std::string error;
    if (!app.initialize(config, error)) {
        std::cerr << "SeriesDash360: " << error << "\n";
        return 1;
    }

    std::cout << "SeriesDash360 core 0.1\n";
    std::cout << "Platform: " << app.platform().platformName() << "\n";
    std::cout << "Library entries: " << app.library().entries().size() << "\n";
    for (const auto& e : app.library().entries()) {
        std::cout << " - " << e.title << " [" << e.id << "] -> " << e.executable.string() << "\n";
    }
    std::cout << "\nOpen preview/index.html for the Xbox Series-style UI prototype.\n";
    return 0;
}
