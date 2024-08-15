import os
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.files import copy

class bookmark_serviceConan(ConanFile):
    name = "bookmark_service"
    version = "0.0.0"
    license = "MIT"
    author = "Dany"
    url = ""
    description = ""
    topics = ("conan", "cpp")
    settings = {"os", "compiler", "build_type", "arch"}
    options = {"shared": [True, False],
               "fPIC": {True, False}}
    default_options = {"shared": False, "fPIC": False}
    generators = "CMakeDeps"
    package_type = "application"
    
    def configure(self):
        if self.options.shared:
            del self.options.fPIC
    

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.user_presets_path = False
        tc.variables["CMAKE_BUILD_TYPE"] = str(self.settings.build_type).upper()
        tc.generate()
   

    def source(self):
        # self.run("git clone git@github.com:cw118876/grocery.git")
        pass

    def system_requirements(self):
        pass


    def requirements(self):
        self.test_requires("gtest/1.15.0")
    

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        # local installation
        cmake.install()


    def package(self):
        # copy("*.h", dst="include", src="include")
        print(self.source_folder)
        print(self.package_folder)
        copy(self, "*", os.path.join(self.source_folder, "include"), 
             os.path.join(self.package_folder, "include"))
        cmake = CMake(self)
        # package installation
        cmake.install()
        

    def package_info(self):
        self.cpp_info.libs = ["bookmark_service"]
        self.cpp_info.name["cmake_find_package"] = "bookmark_service"
        self.cpp_info.names["cmake_find_package_multi"] = "bookmark_service"
