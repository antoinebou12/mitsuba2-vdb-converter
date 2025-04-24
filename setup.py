import os
import platform
import sys
import sysconfig
import subprocess
from setuptools import setup, find_packages
from setuptools.command.build_ext import build_ext
from pybind11.setup_helpers import Pybind11Extension

# Determine platform
is_windows = platform.system() == "Windows"

# Function to detect vcpkg installation
def find_vcpkg_root():
    """Find the vcpkg root directory."""
    # Check environment variable first
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    if vcpkg_root and os.path.exists(vcpkg_root):
        return vcpkg_root
    
    # Check common locations
    if is_windows:
        common_paths = [
            os.path.join(os.path.expanduser("~"), "vcpkg"),
            os.path.join(os.path.expanduser("~"), "Desktop", "vcpkg"),
            os.path.join(os.path.expanduser("~"), "OneDrive", "Bureau", "vcpkg"),
            os.path.join(os.path.expanduser("~"), "Documents", "vcpkg"),
            os.path.join("C:", "vcpkg"),
            os.path.join("D:", "vcpkg"),
        ]
        
        for path in common_paths:
            if os.path.exists(path):
                return path
    
    return None

# Try to find vcpkg
vcpkg_root = find_vcpkg_root()

# Base include and library directories
include_dirs = [
    ".",
    "ext/openvdb",
    *[p for p in [
        sysconfig.get_path("include"),
        os.path.join(sys.prefix, "include"),
        os.path.join(sys.prefix, "include", f"python{sys.version_info.major}.{sys.version_info.minor}"),
        f"/usr/include/python{sys.version_info.major}.{sys.version_info.minor}",
        f"/usr/local/include/python{sys.version_info.major}.{sys.version_info.minor}",
    ] if p]
]

library_dirs = []
libraries = []

# Add vcpkg paths if found
if vcpkg_root:
    print(f"VCPKG_ROOT: {vcpkg_root}")
    
    # Platform-specific triplet
    triplet = "x64-windows" if is_windows else "x64-linux"
    
    # Add vcpkg include and library directories
    vcpkg_include = os.path.join(vcpkg_root, "installed", triplet, "include")
    include_dirs.append(vcpkg_include)
    
    if is_windows:
        vcpkg_lib = os.path.join(vcpkg_root, "installed", triplet, "lib")
        vcpkg_bin = os.path.join(vcpkg_root, "installed", triplet, "bin")
        library_dirs.extend([vcpkg_lib, vcpkg_bin])
    else:
        vcpkg_lib = os.path.join(vcpkg_root, "installed", triplet, "lib")
        library_dirs.append(vcpkg_lib)
    
    # Check for OpenEXR
    openexr_header = os.path.join(vcpkg_include, "OpenEXR", "half.h")
    if not os.path.exists(openexr_header):
        print(f"OpenEXR NOT found at expected path: {openexr_header}")
        print("WARNING: OpenEXR not found in vcpkg. You may need to install it:")
        install_cmd = f"cd {vcpkg_root} && " + (".\\" if is_windows else "./") + f"vcpkg install openexr:{triplet}"
        print(install_cmd)
    else:
        print(f"Found OpenEXR at {openexr_header}")

# Platform-specific flags and libraries
if is_windows:
    extra_compile_args = ["/std:c++14", "/O2", "/EHsc", "/bigobj", "/DNOMINMAX"]
    # Add defines for OpenVDB
    extra_compile_args.extend(["/DOPENVDB_DLL", "/DOPENVDB_STATICLIB"])
    
    # Windows-specific libraries
    libraries.extend([
        "openvdb", 
        "Half-2_5", "Iex-2_5", "IlmThread-2_5", "Imath-2_5",
        "tbb", "blosc", "zlib"
    ])
else:
    extra_compile_args = ["-std=c++14", "-O3", "-fPIC"]
    # Linux/macOS libraries
    libraries.extend([
        "openvdb", "Half", "Iex", "IlmThread", "Imath",
        "tbb", "blosc", "z"
    ])

print(f"Platform: {platform.system()}")
print(f"Include dirs: {include_dirs}")
print(f"Library dirs: {library_dirs}")
print(f"Libraries: {libraries}")

# Define the extension
ext_modules = [
    Pybind11Extension(
        name="volconv._volconv",
        sources=["volconv.cpp", "pybind_volconv.cpp"],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=extra_compile_args,
    ),
]

# Setup package
setup(
    name="volconv",
    version="0.1.0",
    description="Python bindings for OpenVDB → VOL converter",
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    install_requires=["pybind11>=2.6.0", "numpy"],
    zip_safe=False,
)