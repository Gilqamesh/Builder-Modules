# `m03gsy25j4v7nccgmsdov9ioft_shader`

## Goal

Define and validate immutable, backend-independent shader ASTs and reflection. Describe shader operations and resource requirements with meaning that can be checked before execution; AST construction does not execute shaders or allocate rendering resources.

## Stage interfaces

Vertex position and fragment color are `vector<float, 4>` special outputs,
separate from numbered outputs. Vertex transforms use homogeneous
`matrix<float, 4, 4>` built-ins. Fragment inputs own interpolation metadata;
numbered vertex outputs retain location/type compatibility. Validation of a stage value type does not
establish interpolation support for that type.
