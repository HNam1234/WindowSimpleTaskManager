from conan import ConanFile

class SimpleConan(ConanFile):
    requires = "spdlog/1.12.0"
    generators = "CMakeDeps", "CMakeToolchain"
    settings = "os", "compiler", "build_type", "arch"