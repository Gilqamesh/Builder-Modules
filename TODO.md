# Refactoring TODO

## Filesystem lifecycle

- [ ] Add `temporary_directory_t` to `filesystem`; create a unique directory and remove it with non-throwing, best-effort cleanup.
- [ ] Add `current_path_guard_t` to `filesystem`; restore the original process working directory on destruction and document that working-directory mutation is process-global.
- [ ] Add `staged_output_t` for constructing a previously absent destination:

  ```cpp
  class staged_output_t {
  public:
      explicit staged_output_t(path_t destination);
      ~staged_output_t() noexcept;

      const path_t& path() const noexcept;
      void commit();
  };
  ```

  The staging path must be a unique sibling of the destination. Until a single successful rename in `commit()`, the destination remains absent. Destruction removes only uncommitted staging data. Do not initially support replacing existing destinations because that cannot provide the same portable guarantee.

## Environment and bytes

- [ ] Add `scoped_environment_variable_t` to the environment-owning module; restore the previous value on destruction and document that mutation is process-global.
- [x] Add `byte_stream_t::from_file(path_t)` and `byte_stream_t::write_file(path_t) const`.

## Scope guard

- [ ] Harden the existing `scope_guard_t` rather than adding another abstraction: make it non-copyable, movable, releasable, and require non-throwing cleanup.
- [ ] Move it to `ws0` only if a foundational caller remains after staged filesystem output is available.

## Runtime type-erased array

- [ ] Replace template-based access with a fixed runtime descriptor and non-template operations. Keep the supported element domain trivially copyable.

  ```cpp
  struct type_descriptor_t {
      type_id_t id;
      std::size_t size;
      std::size_t alignment;
  };

  struct element_view_t {
      const type_descriptor_t* type;
      std::span<std::byte> bytes;
  };

  struct const_element_view_t {
      const type_descriptor_t* type;
      std::span<const std::byte> bytes;
  };

  class type_erased_array_t {
  public:
      explicit type_erased_array_t(type_descriptor_t element_type);

      const type_descriptor_t& element_type() const noexcept;
      std::size_t element_count() const noexcept;
      element_view_t at(std::size_t index);
      const_element_view_t at(std::size_t index) const;
      void push_back(const_element_view_t value);
      void clear() noexcept;
  };
  ```

  Store elements in contiguous storage allocated to the descriptor's alignment. Validate descriptor identity and byte size on insertion, bounds on access, and allocate before replacing storage so failed growth leaves the array unchanged. `clear()` preserves the element descriptor. Use no inheritance or virtual dispatch.
