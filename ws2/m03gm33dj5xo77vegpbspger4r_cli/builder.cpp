#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>

extern "C" void phase__source(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t* phase) {
    phase->install_source_tree();
}

extern "C" void phase__interface(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::interface_phase_t* phase) {
    phase->install_headers_from_source();
}

extern "C" void phase__library(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::library_phase_t* phase) {
    const auto sources = phase->install<m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t>();
    std::vector<m03gagbhsujjf63n0w3r2w4q6h_build_phases::phase_base_t::built_t> source_files;

    for (const auto& source_file : m03gagbhsnusi43zogoacgj2ez_filesystem::find(sources.root(), m03gagbhsnusi43zogoacgj2ez_filesystem::find_include_predicate_t::cpp_file, m03gagbhsnusi43zogoacgj2ez_filesystem::find_descend_predicate_t::descend_all)) {
        const auto filename = source_file.path().filename();
        if (filename == "builder.cpp" || filename == "cli.cpp") {
            continue;
        }

        source_files.push_back(phase->build(source_file));
    }

    if (!source_files.empty()) {
        const auto library = phase->build_library(source_files, {});
        phase->install_library(library);
    }
}

extern "C" void phase__binary(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::binary_phase_t* phase) {
    phase->install_cli({});
}
