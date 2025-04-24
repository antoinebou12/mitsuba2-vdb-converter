#include "volconv.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

using Path = std::filesystem::path;

// === CLI parsing ===
struct Args {
    Path        inputFile;
    std::string gridName;
    VolFormat   format = VolFormat::BINARY;
};

std::optional<Args> parseArgs(int argc, char* argv[]) {
    if (argc < 2 || argc > 4) return std::nullopt;
    Args a;
    a.inputFile = argv[1];
    if (argc >= 3) a.gridName = argv[2];
    if (argc == 4) {
        std::string f = argv[3];
        if      (f == "ascii")  a.format = VolFormat::ASCII;
        else if (f == "binary") a.format = VolFormat::BINARY;
        else                    return std::nullopt;
    }
    return a;
}

void printUsage() {
    std::cerr << "Usage: volconv <file.vdb> [gridName] [ascii|binary]\n";
}

// === Binary‐writing helper (could also live in volconv.hpp) ===
template<typename T>
void writeBinary(std::ofstream &out, T x) {
    out.write(reinterpret_cast<const char*>(&x), sizeof(T));
}

// === Main ===
int main(int argc, char* argv[]) {
    auto maybe = parseArgs(argc, argv);
    if (!maybe) {
        printUsage();
        return EXIT_FAILURE;
    }
    auto args = *maybe;

    try {
        // call into your refactored library
        auto [hdr, voxels] = convertVDB(
            args.inputFile.string(),
            args.gridName,
            args.format
        );

        // decide output path
        Path outName = args.inputFile.replace_extension(".vol");

        if (args.format == VolFormat::ASCII) {
            std::ofstream out(outName);
            if (!out) throw std::runtime_error("Cannot open " + outName.string());
            for (size_t i = 0; i < voxels.size(); ++i) {
                if (i) out << ", ";
                out << voxels[i];
            }
        }
        else {
            std::ofstream out(outName, std::ios::binary);
            if (!out) throw std::runtime_error("Cannot open " + outName.string());

            // write VOL header (v3)
            out.write("VOL", 3);
            uint8_t version = 3;              writeBinary(out, version);
            int32_t type = 1;                 writeBinary(out, type);      // float
            writeBinary(out, int32_t(hdr.dim.x() - 1));
            writeBinary(out, int32_t(hdr.dim.y() - 1));
            writeBinary(out, int32_t(hdr.dim.z() - 1));
            writeBinary(out, int32_t(1));     // channels

            // world extents
            writeBinary(out, float(hdr.wsMin.x()));
            writeBinary(out, float(hdr.wsMin.y()));
            writeBinary(out, float(hdr.wsMin.z()));
            writeBinary(out, float(hdr.wsMax.x()));
            writeBinary(out, float(hdr.wsMax.y()));
            writeBinary(out, float(hdr.wsMax.z()));

            // voxel data
            for (float v : voxels) {
                writeBinary(out, v);
            }
        }

        std::cout << "Wrote " << outName << " (" << voxels.size() << " voxels)\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
