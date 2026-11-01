#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

extern "C" void phase__interface(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::interface_phase_t* phase) {
    namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
    const auto sources = phase->install<m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t>();
    const filesystem::relative_path_t relative_path("api.h");
    phase->install_interface(phase->build_interface_as(sources.root() / relative_path, relative_path));
}

extern "C" void phase__library(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::library_phase_t* phase) {
    phase->install_library(phase->build_library({phase->source("api.cpp"), phase->source("helpers.cpp")}, {}));
    phase->validate_library("public_api", {phase->source("test/public_api.cpp")});
    phase->validate_library("record_transforms", {phase->source("test/record_transforms.cpp")});
    phase->validate_library("concurrency", {phase->source("test/concurrency.cpp")});
    phase->validate_library("identities", {phase->source("test/identities.cpp")});
    phase->validate_library("standalone_header", {phase->source("test/standalone_header.cpp")});
    phase->validate_library("readme_example", {phase->source("test/readme_example.cpp")});
}

extern "C" void phase__binary(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::binary_phase_t* phase) {
    if (phase->should_install_target("validate")) {
        phase->install_binary("validate", {phase->source("test/standalone_header.cpp")});
    }
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations
