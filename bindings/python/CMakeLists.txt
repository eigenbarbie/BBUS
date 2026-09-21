pybind11_add_module(
  _belief_update
  MODULE
  analysis_bindings.cpp
  core_bindings.cpp
  model_bindings.cpp
  module.cpp
  provenance_bindings.cpp
  resolver_bindings.cpp
  result_bindings.cpp
  replay_bindings.cpp
  stress_bindings.cpp
  tree_bindings.cpp
  update_bindings.cpp
  validation_bindings.cpp
  value_bindings.cpp
)
target_link_libraries(
  _belief_update
  PRIVATE
    belief_update::local_sensitivity
    belief_update::posterior_comparison
    belief_update::threshold_analysis
    belief_update::core
    belief_update::model
    belief_update::replay
    belief_update::resolver
    belief_update::stress
    belief_update::tree
    belief_update::update
)
belief_update_apply_project_options(_belief_update)

set_target_properties(
  _belief_update
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python/belief_update"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python/belief_update"
)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/python/belief_update")
configure_file(
  "${PROJECT_SOURCE_DIR}/python/belief_update/__init__.py"
  "${PROJECT_BINARY_DIR}/python/belief_update/__init__.py"
  COPYONLY
)

install(
  TARGETS _belief_update
  LIBRARY DESTINATION belief_update
  RUNTIME DESTINATION belief_update
)
install(
  FILES "${PROJECT_SOURCE_DIR}/python/belief_update/__init__.py"
  DESTINATION belief_update
)
