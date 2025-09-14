from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class TauCOMRecipe(ConanFile):
    name = "taucom"
    package_type = "library"

    # Optional metadata
    license = ""
    author = "Hyfloac mail@hyfloac.com"
    url = "https://github.com/hyfloac/TauCOM"
    description = ""

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [ True, False ],
        "useTauUtils": [ True, False ]
    }
    default_options = {
        "shared": True,
        "useTauUtils": False
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "cmake/*", "src/*", "include/*"

    def set_version(self):
        self.version = self.conan_data["latest"]

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def requirements(self):
        if self.options.useTauUtils:
            self.requires("tauutils/[^1.3.3]")

    def configure(self):
        return

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["USE_TAU_UTILS"] = self.options.useTauUtils
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["TauCOM"]
    

    

    

