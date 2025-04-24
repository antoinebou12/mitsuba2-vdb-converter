#include "volconv.hpp"
#include <openvdb/tools/Interpolation.h>
#include <stdexcept>
#include <sstream>

std::pair<VolHeader, std::vector<float>>
convertVDB(const std::string& filename,
           const std::string& gridName,
           VolFormat format,
           ProgressCallback progress)
{
    // Initialize OpenVDB
    openvdb::initialize();
    
    // Open the VDB file
    openvdb::io::File file(filename);
    try {
        file.open();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to open VDB file: " + filename + " (" + e.what() + ")");
    }
    
    // Find the grid
    openvdb::GridBase::Ptr base;
    std::string selectedGridName;
    
    // List available grids for error reporting
    std::vector<std::string> gridNames;
    for (auto it = file.beginName(); it != file.endName(); ++it) {
        gridNames.push_back(it.gridName());
        
        if (gridName.empty() || it.gridName() == gridName) {
            try {
                base = file.readGrid(it.gridName());
                selectedGridName = it.gridName();
                if (!base) continue;
                if (!gridName.empty()) break;
            } catch (const std::exception& e) {
                // Continue trying other grids if this one fails
                continue;
            }
        }
    }
    
    if (!base) {
        std::ostringstream oss;
        oss << "Grid '" << (gridName.empty() ? "default" : gridName) << "' not found in " << filename;
        
        if (!gridNames.empty()) {
            oss << "\nAvailable grids: ";
            for (size_t i = 0; i < gridNames.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << gridNames[i];
            }
        }
        
        throw std::runtime_error(oss.str());
    }
    
    // Cast to FloatGrid
    auto grid = openvdb::gridPtrCast<openvdb::FloatGrid>(base);
    if (!grid) {
        throw std::runtime_error("Grid '" + selectedGridName + 
                               "' is not a FloatGrid (type: " + base->type() + ")");
    }
    
    // Get bounding information
    auto dim = grid->evalActiveVoxelDim();
    auto bbox = grid->evalActiveVoxelBoundingBox();
    auto wsMin = grid->indexToWorld(bbox.min());
    auto wsMax = grid->indexToWorld(bbox.max() - openvdb::Vec3R(1));
    
    // Create a grid sampler with the specified interpolation
    openvdb::tools::GridSampler<decltype(*grid), openvdb::tools::BoxSampler> sampler(*grid);
    
    // Calculate dimensions
    int nx = bbox.dim().x(), ny = bbox.dim().y(), nz = bbox.dim().z();
    size_t totalVoxels = static_cast<size_t>(nx) * ny * nz;
    
    // Check if the volume is too large
    constexpr size_t MAX_SAFE_VOXELS = 512*512*512; // ~500MB for float data
    if (totalVoxels > MAX_SAFE_VOXELS) {
        std::ostringstream oss;
        oss << "Volume is very large (" << nx << "x" << ny << "x" << nz << " = " 
            << totalVoxels << " voxels, ~" << (totalVoxels * sizeof(float)) / (1024*1024) 
            << "MB). Continue anyway? Use streamToVol() for better memory efficiency.";
        // If this was interactive, we'd ask for confirmation here
    }
    
    // Allocate memory for voxels
    std::vector<float> voxels;
    voxels.reserve(totalVoxels);
    
    // Extract voxel data with progress reporting
    size_t processedVoxels = 0;
    
    // Determine update frequency for progress reporting
    size_t updateInterval = std::max<size_t>(1, totalVoxels / 100);
    
    for (int k = bbox.min().z(); k < bbox.max().z(); ++k) {
        for (int j = bbox.min().y(); j < bbox.max().y(); ++j) {
            for (int i = bbox.min().x(); i < bbox.max().x(); ++i) {
                // Sample the grid at this position
                voxels.push_back(sampler.isSample({float(i), float(j), float(k)}));
                
                // Update progress if callback provided
                if (progress && (++processedVoxels % updateInterval == 0)) {
                    float percentage = static_cast<float>(processedVoxels) / totalVoxels;
                    progress(percentage);
                }
            }
        }
    }
    
    // Final progress update
    if (progress) {
        progress(1.0f);
    }
    
    // Return the header and voxel data
    VolHeader header = {dim, wsMin, wsMax};
    return {header, std::move(voxels)};
}