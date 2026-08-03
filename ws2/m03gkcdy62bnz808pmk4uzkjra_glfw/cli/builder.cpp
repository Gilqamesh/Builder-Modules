#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsmhr0naw0zpccv4gaq_cxx_toolchain/cxx_toolchain.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>
#include <m03gagbht3svcx3ign454lfup3_cmake/cmake.h>
#include <m03gagbht7wqhtdg9hwdpmfn5o_download/download.h>
#include <m03gagbht9a02hx1qrv2qfgnp7_gzip/gzip.h>
#include <m03gagbhteldyu7ptbgnvootmb_tar/tar.h>

extern "C" void phase__source(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t* phase) {
    phase->install_source_tree();

    const auto glfw_tar_gz = m03gagbht7wqhtdg9hwdpmfn5o_download::fetch(
        m03gagbht7wqhtdg9hwdpmfn5o_download::source_lock_t {
            .url = "https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz",
            .sha256 = "c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01"
        },
        phase->build_dir() / m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("glfw-3.4.tar.gz")
    );

    const auto glfw_tar = m03gagbht9a02hx1qrv2qfgnp7_gzip::ungzip(
        glfw_tar_gz,
        phase->build_dir() / m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("glfw-3.4.tar")
    );

    const auto glfw_extract_dir = m03gagbhteldyu7ptbgnvootmb_tar::untar(
        glfw_tar,
        phase->build_dir() / m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("upstream")
    );
    const auto glfw_source_dir = glfw_extract_dir / m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("glfw-3.4");
    phase->install_source(glfw_source_dir);
}

extern "C" void phase__interface(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::interface_phase_t* phase) {
    const auto sources = phase->install<m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t>();

    for (const auto& header : m03gagbhsnusi43zogoacgj2ez_filesystem::find(
        sources.root(),
        m03gagbhsnusi43zogoacgj2ez_filesystem::find_include_predicate_t::h_file ||
            m03gagbhsnusi43zogoacgj2ez_filesystem::find_include_predicate_t::hpp_file,
        m03gagbhsnusi43zogoacgj2ez_filesystem::find_descend_predicate_t::descend_all
    )) {
        if (header.path().filename().starts_with("cli_")) {
            continue;
        }
        phase->install_interface(header);
    }

    const std::vector<m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t> headers = {
        m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("GLFW/glfw3.h"),
        m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("GLFW/glfw3native.h")
    };

    for (const auto& header : headers) {
        const auto& interface_header = phase->build_interface_as(
            sources.root() /
                m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t(
                    std::format(
                        "upstream/glfw-3.4/include/{}",
                        header.string()
                    )
                ),
            header
        );
        phase->install_interface(interface_header);
    }
}

extern "C" void phase__library(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::library_phase_t* phase) {
    const auto sources = phase->install<m03gagbhsujjf63n0w3r2w4q6h_build_phases::source_phase_t>();
    const auto glfw_source_dir = sources.root() / m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("upstream/glfw-3.4");
    const auto library_build_dir = phase->build_dir();
    const auto cmake_build_dir = library_build_dir /
        m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t(
            "m03gagbht3svcx3ign454lfup3_cmake"
        );

    if (phase->library_type() != m03gagbhsmhr0naw0zpccv4gaq_cxx_toolchain::library_type_t::SHARED) {
        throw std::runtime_error("phase__library: does not support static library type for glfw");
    }

    m03gagbht3svcx3ign454lfup3_cmake::configure(glfw_source_dir, cmake_build_dir, {
        { "BUILD_SHARED_LIBS", "ON" },
        { "CMAKE_INSTALL_PREFIX", library_build_dir.string() },
        { "CMAKE_BUILD_TYPE", "Debug" },
        { "GLFW_BUILD_EXAMPLES", "OFF" },
        { "GLFW_BUILD_TESTS", "OFF" },
        { "GLFW_BUILD_DOCS", "OFF" }
    });
    m03gagbht3svcx3ign454lfup3_cmake::build(cmake_build_dir, std::nullopt);
    m03gagbht3svcx3ign454lfup3_cmake::install(cmake_build_dir);

    bool installed_library = false;
    for (const auto& artifact : m03gagbhsnusi43zogoacgj2ez_filesystem::find(
        library_build_dir,
        !m03gagbhsnusi43zogoacgj2ez_filesystem::find_include_predicate_t::is_dir,
        m03gagbhsnusi43zogoacgj2ez_filesystem::find_descend_predicate_t(
            [&](const m03gagbhsnusi43zogoacgj2ez_filesystem::path_t& dir, std::size_t) {
                return dir != cmake_build_dir;
            }
        )
    )) {
        const auto artifact_path = artifact.path();
        const auto filename = artifact_path.filename();
        if (filename.starts_with("libglfw") && (filename.find(".so") != std::string::npos)) {
            phase->install_library(artifact_path);
            installed_library = true;
        }
    }
    if (!installed_library) {
        throw std::runtime_error(std::format(
            "libraries/m03gkcdy62bnz808pmk4uzkjra_glfw: "
            "expected glfw library under '{}'",
            library_build_dir
        ));
    }

    const auto readline_so_as = m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("libreadline.so");
    const auto readline_so_path = m03gagbhsnusi43zogoacgj2ez_filesystem::path_t("/usr/lib64") / readline_so_as;
    const auto readline_so = phase->build(readline_so_path, readline_so_as);
    phase->install_library(readline_so);

    const auto history_so_as = m03gagbhsnusi43zogoacgj2ez_filesystem::relative_path_t("libhistory.so");
    const auto history_so_path = m03gagbhsnusi43zogoacgj2ez_filesystem::path_t("/usr/lib64") / history_so_as;
    const auto history_so = phase->build(history_so_path, history_so_as);
    phase->install_library(history_so);

    std::vector<m03gagbhsujjf63n0w3r2w4q6h_build_phases::phase_base_t::built_t> source_files;
    for (const auto& source_file : m03gagbhsnusi43zogoacgj2ez_filesystem::find(
        sources.root(),
        m03gagbhsnusi43zogoacgj2ez_filesystem::find_include_predicate_t::cpp_file,
        m03gagbhsnusi43zogoacgj2ez_filesystem::find_descend_predicate_t::descend_all
    )) {
        const auto filename = source_file.path().filename();
        if (filename == "builder.cpp" || filename == "cli.cpp" || filename.starts_with("cli_")) {
            continue;
        }

        source_files.push_back(phase->build(source_file));
    }

    const auto library = phase->build_library(source_files, {});
    phase->install_library(library);
}

extern "C" void phase__binary(const m03gagbhsujjf63n0w3r2w4q6h_build_phases::binary_phase_t* phase) {
    phase->install_cli({});
}
