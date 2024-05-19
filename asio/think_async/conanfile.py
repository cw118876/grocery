import os
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain

class MyLibraryConan(ConanFile):
    name = "think_async"
    version = "0.0.1"
    license = "MIT"
    author = "Your Name"
    url = "https://github.com/cw118876/grocery.git"
    description = "A sample Conan package for Think-Async"
    topics = ("conan", "cpp", "asio")
    settings = {"os", "compiler", "build_type", "arch"}
    options = {"shared": [True, False],
               "fPIC": {True, False}}
    default_options = {"shared": False, "fPIC": False}
    generators = "CMakeDeps"
    
    def configure(self):
        if self.options.shared:
            del self.options.fPIC
    

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.user_presets_path = False
        tc.variables["CMAKE_BUILD_TYPE"] = str(self.settings.build_type).upper()
        tc.generate()
   

    def source(self):
        self.run("git clone git@github.com:cw118876/grocery.git")

    def system_requirements(self):
        pass


    def requirements(self):
        self.requires("asio/1.30.2")
    

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        # local installation
        cmake.install()


    def package(self):
        # copy("*.h", dst="include", src="include")
        cmake = CMake(self)
        # package installation
        cmake.install()
        

    def package_info(self):
        self.cpp_info.libs = ["Think-Async"]
        self.cpp_info.name["cmake_find_package"] = "Think-Async"
