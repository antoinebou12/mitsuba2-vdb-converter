# mitsuba2-vdb-converter
A small utility to convert OpenVDB files to the Mitsuba 2 volume format.

## Compiling
The converter uses a recursive CMake setup to compile OpenVDB and its dependencies. The repository should be cloned recursively, i.e.
```
git clone --recursive https://github.com/mitsuba-renderer/mitsuba2-vdb-converter.git
```
Then, the build process is the regular CMake process:
```
cd mitsuba2-vdb-converter
mkdir build
cd build
cmake ..
make -j
```
This should then build all the dependencies as well as the converter utility. The setup was tested on Arch Linux and macOS 10.14.

## Command-line Usage

Currently, the usage is limited to extracting a single float grid from an OpenVDB file.
In the simplest case, the converter can be used as
```
./convertvdb myvolumefile.vdb
```
The output is then written in the same path with the suffix `.vol`. If the OpenVDB file contains multiple grids, one of them can be selected:
```
./convertvdb myvolumefile.vdb gridName
```
To get a list of grids in a file, one can use the `vdb_print` program, which is part of OpenVDB. In this CMake setup, the `vdb_print` executable is built and written to a subfolder in `build/ext_build`.

## Python Package

The converter is also available as a Python package, which makes it easy to integrate with existing Python workflows.

### Installation

After successfully building the C++ components as described above, install the Python package:

```bash
pip install .
```

If OpenVDB is installed in a non-standard location, you may need to set:

```bash
export OPENVDB_INC=/path/to/openvdb/include
export OPENVDB_LIB=/path/to/openvdb/lib
```

### Python Usage

The Python API provides a simple interface to convert OpenVDB files to NumPy arrays:

```python
import volconv
import numpy as np

# Convert a VDB file to a NumPy array
volume_data = volconv.convert("myvolumefile.vdb")

# The result is a 3D NumPy array with shape (Z, Y, X)
print(f"Volume shape: {volume_data.shape}")
```

## Known issues

As of December 2019, there is an issue with Homebrew's boost installation on MacOS.
This can be fixed by following https://stackoverflow.com/a/55828190.

### Python Installation Issues
If you encounter errors during the Python package installation, try:
- Using the `--use-pep517` flag: `pip install . --use-pep517`
- Upgrading setuptools: `pip install --upgrade setuptools`
- Installing without editable mode: `pip install .` (instead of `pip install -e .`)