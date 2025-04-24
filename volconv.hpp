#pragma once

#include <openvdb/openvdb.h>
#include <string>
#include <vector>
#include <utility>

// Output format enum
enum class VolFormat {
    ASCII,
    BINARY
};

// Volumetric data header information
struct VolHeader {
    openvdb::Coord dim;     // Grid dimensions
    openvdb::Vec3R wsMin;   // World-space minimum coordinates
    openvdb::Vec3R wsMax;   // World-space maximum coordinates
};

// Main conversion function
std::pair<VolHeader, std::vector<float>>
convertVDB(const std::string& filename,
           const std::string& gridName = "",
           VolFormat format = VolFormat::BINARY);