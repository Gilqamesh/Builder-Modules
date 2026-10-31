#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>
#include <m03gagbht7wqhtdg9hwdpmfn5o_download/download.h>
#include <m03gagbht9a02hx1qrv2qfgnp7_gzip/gzip.h>
#include <m03gagbhteldyu7ptbgnvootmb_tar/tar.h>

#include <initializer_list>

namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite {

extern "C" void phase__source(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t* phase) {
    namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
    phase->install_source_tree();

    const auto archive = m03gagbht7wqhtdg9hwdpmfn5o_download::fetch(
        {
            .url = "https://www.sqlite.org/2026/sqlite-autoconf-3540100.tar.gz",
            .checksum = {m03gagbht7wqhtdg9hwdpmfn5o_download::checksum_algorithm_t::sha3_256, "b6ced585c750916b1052667239a12c25a7ff34e7f06e1e6e01ecb0427657d9c4"}
        },
        phase->build_dir() / filesystem::relative_path_t("sqlite-autoconf-3540100.tar.gz")
    );
    const auto tar = m03gagbht9a02hx1qrv2qfgnp7_gzip::ungzip(
        archive,
        phase->build_dir() / filesystem::relative_path_t("sqlite-autoconf-3540100.tar")
    );
    const auto upstream = m03gagbhteldyu7ptbgnvootmb_tar::untar(
        tar,
        phase->build_dir() / filesystem::relative_path_t("upstream")
    ) / filesystem::relative_path_t("sqlite-autoconf-3540100");

    for (const auto filename : { "sqlite3.c", "sqlite3.h", "sqlite3ext.h" }) {
        const auto relative_path = filesystem::relative_path_t(filename);
        const auto staged = phase->build_dir() / relative_path;
        filesystem::copy(upstream / relative_path, staged);
        phase->install_source(staged);
    }
}

extern "C" void phase__interface(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::interface_phase_t* phase) {
    namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
    const auto sources = phase->install<m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t>();
    const auto relative_path = filesystem::relative_path_t("sqlite.h");
    const auto header = phase->build_interface_as(sources.root() / relative_path, relative_path);
    phase->install_interface(header);
}

extern "C" void phase__library(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::library_phase_t* phase) {
    const auto library = phase->build_library({ phase->source("sqlite3.c"), phase->source("sqlite.cpp"), phase->source("helpers.cpp") }, {});
    phase->install_library(library);
}

} // namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite
