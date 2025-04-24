import os
from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext

# assume your CMake build has already been run, or you can
# call it via a custom build_py step. For simplicity we
# rely on setuptools to compile the extension in-place here.

ext_modules = [
    Pybind11Extension(
        name="volconv._volconv",
        # compile both your library and the pybind glue
        sources=[
            "volconv.cpp",
            "pybind_volconv.cpp",
        ],
        include_dirs=[
            ".",              # for volconv.hpp
            os.environ.get("OPENVDB_INC", "/usr/local/include")
        ],
        library_dirs=[
            os.environ.get("OPENVDB_LIB", "/usr/local/lib")
        ],
        libraries=[
            "openvdb", "tbb", "blosc", "boost_system", "z",
        ],
        extra_compile_args=["-std=c++14", "-O3", "-march=native"],
        extra_link_args=[],
    ),
]

setup(
    name="volconv",
    version="0.1.0",
    description="Python bindings for OpenVDB → VOL converter",
    author="Your Name",
    packages=find_packages(),  # will pick up the volconv/ folder
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    install_requires=["pybind11>=2.6.0", "numpy"],
    zip_safe=False,
)
