from conan import ConanFile

class SimpleConan(ConanFile):
    name = "window_simple_task_manager"
    version = "0.1"

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        "spdlog/1.12.0",
    )

    generators = (
        "CMakeDeps",
        "CMakeToolchain",
    )
