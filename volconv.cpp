#include "volconv.hpp"
#include <openvdb/tools/Interpolation.h>

std::pair<VolHeader, std::vector<float>>
convertVDB(const std::string& filename,
           const std::string& gridName,
           VolFormat /*format*/)
{
    openvdb::initialize();
    openvdb::io::File file(filename);
    file.open();

    // pick grid
    openvdb::GridBase::Ptr base;
    for (auto it = file.beginName(); it != file.endName(); ++it) {
        if (gridName.empty() || it.gridName()==gridName) {
            base = file.readGrid(it.gridName());
            if (!base) continue;
            if (!gridName.empty()) break;
        }
    }
    if (!base) throw std::runtime_error("Grid not found");

    auto grid = openvdb::gridPtrCast<openvdb::FloatGrid>(base);
    if (!grid) throw std::runtime_error("Not a FloatGrid");

    auto dim   = grid->evalActiveVoxelDim();
    auto bbox  = grid->evalActiveVoxelBoundingBox();
    auto wsMin = grid->indexToWorld(bbox.min());
    auto wsMax = grid->indexToWorld(bbox.max() - openvdb::Vec3R(1));

    openvdb::tools::GridSampler<decltype(*grid), openvdb::tools::BoxSampler> sampler(*grid);

    int nx = bbox.dim().x(), ny = bbox.dim().y(), nz = bbox.dim().z();
    std::vector<float> voxels;
    voxels.reserve(nx*ny*nz);

    for (int k=bbox.min().z(); k<bbox.max().z(); ++k)
    for (int j=bbox.min().y(); j<bbox.max().y(); ++j)
    for (int i=bbox.min().x(); i<bbox.max().x(); ++i)
        voxels.push_back( sampler.isSample({float(i),float(j),float(k)}) );

    return { {dim, wsMin, wsMax}, std::move(voxels) };
}
