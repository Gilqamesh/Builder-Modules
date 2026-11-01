# Raylib ImGui backend

## Goal

Connect Dear ImGui input and drawing to Raylib so applications can place interactive UI within a Raylib frame loop. This module owns backend setup, frame integration, and UI rendering; ImGui owns widgets and applications own interface content and frame sequencing.

See [rlImGui.h](rlImGui.h) for the public interface.
