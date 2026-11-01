#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

extern "C" void phase__interface(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::interface_phase_t* phase) {
    namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
    const auto sources = phase->install<m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t>();
    const filesystem::relative_path_t relative_path("database.h");
    phase->install_interface(phase->build_interface_as(sources.root() / relative_path, relative_path));
}

extern "C" void phase__library(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::library_phase_t* phase) {
    phase->install_library(phase->build_library({ phase->source("database.cpp") }, {}));
    phase->validate_library("transactions", { phase->source("test/transactions.cpp") });
    phase->validate_library("private_backend", { phase->source("test/backend.cpp"), phase->source("test/faults.cpp") });
    phase->validate_library("standalone_header", { phase->source("test/standalone_header.cpp") });
    phase->validate_library("readme_example", { phase->source("test/readme_example.cpp") });
    phase->validate_library("client_codecs", { phase->source("test/client_codecs.cpp") });
}

extern "C" void phase__binary(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::binary_phase_t* phase) {
    if (phase->should_install_target("validate")) {
        phase->install_binary("validate", { phase->source("test/standalone_header.cpp") });
    }
}

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
