add_executable(belief_update_demo belief_update_demo.cpp)
target_link_libraries(
  belief_update_demo
  PRIVATE
    belief_update::core
    belief_update::local_sensitivity
    belief_update::posterior_comparison
    belief_update::replay
    belief_update::stress
    belief_update::threshold_analysis
    belief_update::update
)
belief_update_apply_project_options(belief_update_demo)
