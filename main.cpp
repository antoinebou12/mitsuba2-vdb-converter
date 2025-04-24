#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/ValueTransformer.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

// 1) nicer enums
enum class Format { ASCII, BINARY };

struct Args {
    std::filesystem::path  inputFile;
    std::string            gridName;
    Format                 format = Format::BINARY;
};

// improved CLI parsing
std::optional<Args> parseArgs(int argc, char* argv[]) {
    if (argc < 2 || argc > 4) return std::nullopt;
    Args a;
    a.inputFile = argv[1];
    if (argc >= 3) a.gridName = argv[2];
    if (argc == 4) {
        std::string f = argv[3];
        if (f == "ascii") a.format = Format::ASCII;
        else if (f == "binary") a.format = Format::BINARY;
        else return std::nullopt;
    }
    return a;
}

void printUsage() {
    std::cerr << "Usage: volconv <file.vdb> [gridName] [ascii|binary]\n";
}

template<typename T>
void writeBinary(std::ofstream &f, T x) {
    f.write(reinterpret_cast<const char*>(&x), sizeof(T));
}

// emit VOL header (v3) — easy to extend to N channels
void writeVOLHeader(std::ofstream &out,
                    const openvdb::Vec3i& dim,
                    const openvdb::Vec3R& wsMin,
                    const openvdb::Vec3R& wsMax)
{
    out.write("VOL", 3);
    uint8_t version = 3; writeBinary(out, version);

    int32_t type         = 1; // float
    int32_t nx = dim.x()-1,
            ny = dim.y()-1,
            nz = dim.z()-1,
            nch = 1;
    writeBinary(out, type);
    writeBinary(out, nx);
    writeBinary(out, ny);
    writeBinary(out, nz);
    writeBinary(out, nch);

    // world extents
    writeBinary(out, float(wsMin.x())); writeBinary(out, float(wsMin.y())); writeBinary(out, float(wsMin.z()));
    writeBinary(out, float(wsMax.x())); writeBinary(out, float(wsMax.y())); writeBinary(out, float(wsMax.z()));
}

int main(int argc, char* argv[]) {
    auto maybe = parseArgs(argc, argv);
    if (!maybe) {
        printUsage();
        return EXIT_FAILURE;
    }
    auto [inputFile, gridName, format] = *maybe;

    openvdb::initialize();
    openvdb::io::File file(inputFile.string());
    file.open();

    // 2) pick the grid
    openvdb::GridBase::Ptr base;
    for (auto it = file.beginName(); it != file.endName(); ++it) {
        if (gridName.empty() || it.gridName() == gridName) {
            base = file.readGrid(it.gridName());
            if (!base) continue;
            if (!gridName.empty()) break;
        }
    }
    if (!base) {
        std::cerr << "Grid \""<< gridName <<"\" not found in " << inputFile << "\n";
        return EXIT_FAILURE;
    }

    auto grid = openvdb::gridPtrCast<openvdb::FloatGrid>(base);
    if (!grid) {
        std::cerr << "Grid is not a FloatGrid.\n";
        return EXIT_FAILURE;
    }

    // 3) get bounds & sampler
    auto dim  = grid->evalActiveVoxelDim();
    auto bbox = grid->evalActiveVoxelBoundingBox();
    auto wsMin = grid->indexToWorld(bbox.min()),
         wsMax = grid->indexToWorld(bbox.max() - openvdb::Vec3R(1));

    openvdb::tools::GridSampler<decltype(*grid), openvdb::tools::BoxSampler> sampler(*grid);

    // 4) gather samples
    int nx = bbox.dim().x(),
        ny = bbox.dim().y(),
        nz = bbox.dim().z();
    std::vector<float> values;
    values.reserve(nx * ny * nz);

    for (int k = bbox.min().z(); k < bbox.max().z(); ++k) {
      for (int j = bbox.min().y(); j < bbox.max().y(); ++j) {
        for (int i = bbox.min().x(); i < bbox.max().x(); ++i) {
          values.push_back(sampler.isSample({float(i), float(j), float(k)}));
        }
      }
    }

    // 5) write out
    auto outName = inputFile.replace_extension(".vol");
    if (format == Format::ASCII) {
        std::ofstream out(outName);
        for (size_t i = 0; i < values.size(); ++i) {
            if (i) out << ", ";
            out << values[i];
        }
    }
    else {
        std::ofstream out(outName, std::ios::binary);
        writeVOLHeader(out, dim, wsMin, wsMax);
        for (float v : values) writeBinary(out, v);
    }

    std::cout << "Wrote " << outName << " (" << values.size() << " voxels)\n";
    return EXIT_SUCCESS;
}
