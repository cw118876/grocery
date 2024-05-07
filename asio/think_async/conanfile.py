from conans import ConanFile, CMake, tools

class MyLibraryConan(ConanFile):
    name = "think_async"
    version = "1.0"
    license = "MIT"
    author = "Your Name"
    url = "https://github.com/cw118876/grocery.git"
    description = "A sample Conan package for Think-Async"
    topics = ("conan", "cpp", "asio")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False],
               "fPIC": {True, False}}
    default_options = {"shared": False, "fPIC": False}
    generators = "cmake"
    
    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def source(self):
        self.run("git clone git@github.com:cw118876/grocery.git")

    def system_requirements(self):
        pass

    def requirements(self):
        self.requires("asio/1.30.2")
    

    def build(self):
        cmake = CMake(self, generator="Ninja")
        cmake.verbose = True
        cmake.configure(source_folder=self.source_folder)
        cmake.build()
        cmake.install()

    def package(self):
        self.copy("*.h", dst="include", src="include")
        

    def package_info(self):
        self.cpp_info.libs = ["Think-Async"]
        self.cpp_info.name["cmake_find_package"] = "Think-Async"
