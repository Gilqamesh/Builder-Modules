#include <m03gagbhsujjf63n0w3r2w4q6h_build_phases/build_phases.h>
#include <m03gagbhsp2drqq3gkop8pzfrm_workspace_graph/workspace_graph.h>

#include <exception>
#include <format>
#include <iostream>

int main() {
    try {
        namespace graph = m03gagbhsp2drqq3gkop8pzfrm_workspace_graph;
        namespace phases = m03gagbhsujjf63n0w3r2w4q6h_build_phases;
        const auto context = graph::invocation_context();
        graph::workspace_graph_t workspace_graph(context.workspace_root, context.artifact_root);
        auto* module = workspace_graph.discover_module(graph::module_name_t("m03h5hm86mnvc18opf1rdttsgh_database"));
        const auto phase = phases::phase_base_t::make(*module);
        const auto installed = phase->install<phases::library_phase_t>();
        std::cout << std::format("{}\n", installed.root());
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
