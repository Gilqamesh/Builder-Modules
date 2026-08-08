#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>

namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
namespace build_phases = m03gagbhsujjf63n0w3r2w4q6h_build_phases;

extern "C" void phase__library(const build_phases::library_phase_t* phase) {
    const auto library = phase->build_library({ phase->source("api.cpp") }, {});
    phase->install_library(library);

    const auto readline_so_as = filesystem::relative_path_t("libreadline.so");
    const auto readline_so_path = filesystem::path_t("/usr/lib64") / readline_so_as;
    phase->install_library(phase->build(readline_so_path, readline_so_as));

    const auto history_so_as = filesystem::relative_path_t("libhistory.so");
    const auto history_so_path = filesystem::path_t("/usr/lib64") / history_so_as;
    phase->install_library(phase->build(history_so_path, history_so_as));
}
