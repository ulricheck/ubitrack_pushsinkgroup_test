from conans import ConanFile, CMake
from conans import tools
from conans.tools import os_info, SystemPackageTool
import os, sys
import sysconfig
from io import StringIO

class ArtekmedP02Conan(ConanFile):
    name = "sinkgroup_test"
    version = "0.1"

    ubitrack_version = "1.3.0"

    description = "sinkgroup_test"
    url = "https://github.com/ulricheck/ubitrack_sinkgroup_test"
    license = "GPL"

    short_paths = True
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "ubitrack_virtualenv_generator"

    requires = (
        "ubitrack/%s@ubitrack/stable" % ubitrack_version,
        "ubitrack_component_vision_aruco/%s@ubitrack/stable" % ubitrack_version,
        "ubitrack_tools_trackman/1.0@ubitrack/stable",
         )

    default_options = {
    }

    # all sources are deployed with the package
    exports_sources = "sinkgroup_test/*", "CMakeLists.txt"

    def configure(self):
        self.options['ubitrack_vision'].opengl_extension_wrapper = "glad"
        self.options['ubitrack'].with_default_camera = True

    def imports(self):
        self.copy(src="bin", pattern="*.dll", dst="./bin") # Copies all dll files from packages bin folder to my "bin" folder
        self.copy(src="lib", pattern="*.dll", dst="./bin") # Copies all dll files from packages bin folder to my "bin" folder
        self.copy(src="lib", pattern="*.dylib*", dst="./lib") # Copies all dylib files from packages lib folder to my "lib" folder
        self.copy(src="lib", pattern="*.so*", dst="./lib") # Copies all so files from packages lib folder to my "lib" folder
        self.copy(src="lib", pattern="*.a", dst="./lib") # Copies all static libraries from packages lib folder to my "lib" folder
        self.copy(src="bin", pattern="*", dst="./bin") # Copies all applications
        self.copy(src="bin", pattern="log4cpp.conf", dst="./") # copy a logging config template
        self.copy(src="share/Ubitrack", pattern="*.*", dst="./share/Ubitrack") # copy all shared ubitrack files
       
    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()
        cmake.install()
