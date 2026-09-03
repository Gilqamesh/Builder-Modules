# Builder-Modules

Public collection of higher-level modules built with
[Builder](https://github.com/Gilqamesh/Builder). This repository does not
contain Builder's bootstrap and foundational modules; it is intended to be
combined with Builder in a separate runtime and development layout.

## Contents

- [Setup](#setup)
- [Notable modules](#notable-modules)
- [Requirements](#requirements)
- [License](#license)

## Setup

Clone Builder and Builder-Modules as sibling directories:

```bash
git clone https://github.com/Gilqamesh/Builder.git Builder
git clone https://github.com/Gilqamesh/Builder-Modules.git Builder-Modules
```

Create a combined layout containing both repositories:

```bash
mkdir -p Builder-Layout/ws0 Builder-Layout/ws1 Builder-Layout/ws2 Builder-Layout/artifacts
cd Builder-Layout

for repository in Builder Builder-Modules; do
    for workspace in ws0 ws1 ws2; do
        source_dir="../$repository/$workspace"
        [ -d "$source_dir" ] || continue

        for module_dir in "$source_dir"/*; do
            [ -d "$module_dir" ] || continue
            module_name=${module_dir##*/}
            ln -sT \
                "../../$repository/$workspace/$module_name" \
                "$workspace/$module_name"
        done
    done
done

make -f ws0/m03gagbhst621faiop1rztfkqp_builder_cli/bootstrap.mk bootstrap
./cli <module-name>[:target] [<args>...]
```

The default target is `cli`. For example:

```bash
./cli m03ge9zyrjajugagmp61034qhi_module_dependency_ir_svg_renderer m03ge9ij4lbns2mq6722cd8654_function_visualizer "$PWD/out.svg"
```

## Notable modules

- `m03gf09la5rvbh6kk4vvt1qawv_module_shell` - interactive shell for running modules by friendly name.
- `m03ge9zyrjajugagmp61034qhi_module_dependency_ir_svg_renderer` - renders a module dependency graph to SVG.
- `m03ge9ij4lbns2mq6722cd8654_function_visualizer` - infinitely zoomable prototype to visualize m03ge9ij46lc986vpdamnc2fka_function_ir.

## Requirements

- [Builder](https://github.com/Gilqamesh/Builder), combined into the same
  workspace layout.
- Builder's platform and toolchain prerequisites.

## License
MIT - see [LICENSE](LICENSE).
