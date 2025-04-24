import os
import platform
import sys
import subprocess
import sysconfig
from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext

# Determine the platform
is_windows = platform.system() == "Windows"

# Custom vcpkg path use os.environ.get('VCPKG_ROOT') or default to current directory
VCPKG_ROOT = os.environ.get('VCPKG_ROOT', os.path.abspath(os.path.dirname(__file__)))

# Check if OpenEXR is installed in vcpkg
def check_openexr_installation():
    openexr_include = os.path.join(VCPKG_ROOT, "installed", "x64-windows", "include", "OpenEXR", "half.h")
    if os.path.exists(openexr_include):
        print(f"OpenEXR found at: {openexr_include}")
        return True
    else:
        print(f"OpenEXR NOT found at expected path: {openexr_include}")
        return False

# Base compiler flags
extra_compile_args = []
extra_link_args = []

# Platform-specific adjustments
if is_windows:
    # MSVC-compatible flags
    extra_compile_args = ["/std:c++14", "/O2", "/EHsc", "/bigobj"]
    
    # Add Windows-specific preprocessor definitions
    extra_compile_args.extend(["/DNOMINMAX", "/DOPENVDB_DLL", "/DOPENVDB_STATICLIB"])
    
    # Add include path definition for OpenEXR
    openexr_inc_path = os.path.join(VCPKG_ROOT, "installed", "x64-windows", "include")
    extra_compile_args.append(f"/I{openexr_inc_path}")
else:
    # GCC/Clang flags
    extra_compile_args = ["-std=c++14", "-O3"]

# Try to find Python include directories
python_include_dirs = [
    sysconfig.get_path("include"),
    os.path.join(sys.prefix, "include"),
    os.path.join(sys.prefix, "include", f"python{sys.version_info.major}.{sys.version_info.minor}"),
]

# Set up include directories - with specific focus on vcpkg paths
include_dirs = [
    ".",
    "ext/openvdb",
    # Explicitly include OpenEXR parent directory to help find OpenEXR/half.h
    os.path.join(VCPKG_ROOT, "installed", "x64-windows", "include"),
]

# Add Python include directories
include_dirs.extend([dir for dir in python_include_dirs if os.path.exists(dir)])

# Check if we have required dependencies
if is_windows:
    if not check_openexr_installation():
        print("WARNING: OpenEXR not found in vcpkg. You may need to install it:")
        print(f"cd {VCPKG_ROOT} && .\\vcpkg install openexr:x64-windows")

# Define source files that need preprocessing
class SourcePreprocessor:
    @staticmethod
    def preprocess_openvdb_types():
        """Modify OpenVDB Types.h to use correct path for half.h"""
        types_h_path = os.path.join(os.getcwd(), "ext", "openvdb", "openvdb", "Types.h")
        if not os.path.exists(types_h_path):
            print(f"Warning: Could not find {types_h_path} for preprocessing")
            return

        with open(types_h_path, 'r') as f:
            content = f.read()
        
        # Replace OpenEXR/half.h with just half.h if needed
        if 'OpenEXR/half.h' in content:
            new_content = content.replace('OpenEXR/half.h', 'half.h')
            try:
                with open(types_h_path, 'w') as f:
                    f.write(new_content)
                print(f"Successfully modified {types_h_path} to use half.h directly")
            except Exception as e:
                print(f"Failed to modify {types_h_path}: {e}")

# Try to preprocess OpenVDB source files
preprocessor = SourcePreprocessor()
preprocessor.preprocess_openvdb_types()

# Libraries to link against
libraries = ["openvdb", "Half-2_5", "Iex-2_5", "IlmThread-2_5", "Imath-2_5", "tbb", "blosc", "zlib"]

# Library directories from vcpkg
library_dirs = [
    os.path.join(VCPKG_ROOT, "installed", "x64-windows", "lib"),
    os.path.join(VCPKG_ROOT, "installed", "x64-windows", "bin"),
]

# Print debug info
print(f"Platform: {platform.system()}")
print(f"VCPKG_ROOT: {VCPKG_ROOT}")
print(f"Include dirs: {include_dirs}")
print(f"Library dirs: {library_dirs}")
print(f"Libraries: {libraries}")

ext_modules = [
    Pybind11Extension(
        name="volconv._volconv",
        sources=["volconv.cpp", "pybind_volconv.cpp"],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        language="c++",
    ),
]

setup(
    name="volconv",
    version="0.1.0",
    description="Python bindings for OpenVDB → VOL converter",
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    install_requires=["pybind11>=2.6.0", "numpy"],
    zip_safe=False,
    python_requires=">=3.7",
)