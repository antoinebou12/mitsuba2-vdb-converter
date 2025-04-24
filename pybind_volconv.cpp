#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "volconv.hpp"

namespace py = pybind11;

PYBIND11_MODULE(volconv, m) {
    m.doc() = "volconv: convert OpenVDB FloatGrid to NumPy arrays";

    py::enum_<VolFormat>(m, "VolFormat")
        .value("ASCII",  VolFormat::ASCII)
        .value("BINARY", VolFormat::BINARY)
        .export_values();

    m.def("convert",
        [](const std::string &fn, const std::string &grid, VolFormat fmt) {
            auto [hdr, vox] = convertVDB(fn, grid, fmt);
            // build a (Z,Y,X) array:
            auto dim = hdr.dim;
            std::vector<ssize_t> shape = { dim.z()-1, dim.y()-1, dim.x()-1 };
            return py::array_t<float>(
                shape,
                { //(strides in bytes)
                  sizeof(float)*shape[1]*shape[2],
                  sizeof(float)*shape[2],
                  sizeof(float)
                },
                vox.data(), // no copy
                /* owner */ nullptr
            );
        },
        py::arg("filename"),
        py::arg("gridname") = "",
        py::arg("format")   = VolFormat::BINARY,
        "Read a FloatGrid and return a 3D NumPy array (Z,Y,X)."
    );
}
