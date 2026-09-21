add_executable(belief_update_cpp_smoke cpp/smoke_test.cpp)
target_link_libraries(belief_update_cpp_smoke PRIVATE belief_update::core)
belief_update_apply_project_options(belief_update_cpp_smoke)

add_test(NAME cpp.smoke COMMAND belief_update_cpp_smoke)
set_tests_properties(cpp.smoke PROPERTIES LABELS "unit")

foreach(
  test_name
  IN ITEMS
    bayes_edge_cases
    bayes_exact
    bayes_log_odds
    probability_types
    provenance
    result_types
)
  add_executable("belief_update_${test_name}_test" "cpp/${test_name}_test.cpp")
  target_link_libraries(
    "belief_update_${test_name}_test"
    PRIVATE belief_update::core
  )
  target_include_directories(
    "belief_update_${test_name}_test"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
  )
  belief_update_apply_project_options("belief_update_${test_name}_test")
  add_test(
    NAME "cpp.${test_name}"
    COMMAND "belief_update_${test_name}_test"
  )
  set_tests_properties("cpp.${test_name}" PROPERTIES LABELS "unit")
endforeach()

foreach(test_name IN ITEMS validator_provenance validator_scalar)
  add_executable("belief_update_${test_name}_test" "cpp/${test_name}_test.cpp")
  target_link_libraries(
    "belief_update_${test_name}_test"
    PRIVATE belief_update::validation
  )
  target_include_directories(
    "belief_update_${test_name}_test"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
  )
  belief_update_apply_project_options("belief_update_${test_name}_test")
  add_test(NAME "cpp.${test_name}" COMMAND "belief_update_${test_name}_test")
  set_tests_properties("cpp.${test_name}" PROPERTIES LABELS "unit;validation")
endforeach()

add_executable(belief_update_input_resolver_test cpp/input_resolver_test.cpp)
target_link_libraries(
  belief_update_input_resolver_test
  PRIVATE belief_update::resolver
)
target_include_directories(
  belief_update_input_resolver_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_input_resolver_test)
add_test(NAME cpp.input_resolver COMMAND belief_update_input_resolver_test)
set_tests_properties(cpp.input_resolver PROPERTIES LABELS "unit;resolver")

add_executable(belief_update_likelihood_model_test cpp/likelihood_model_test.cpp)
target_link_libraries(
  belief_update_likelihood_model_test
  PRIVATE belief_update::core belief_update::model belief_update::update
)
target_include_directories(
  belief_update_likelihood_model_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_likelihood_model_test)
add_test(NAME cpp.likelihood_model COMMAND belief_update_likelihood_model_test)
set_tests_properties(cpp.likelihood_model PROPERTIES LABELS "unit;model")

add_executable(
  belief_update_convenience_update_test
  cpp/convenience_update_test.cpp
)
target_link_libraries(
  belief_update_convenience_update_test
  PRIVATE belief_update::core belief_update::replay belief_update::update
)
target_include_directories(
  belief_update_convenience_update_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_convenience_update_test)
add_test(
  NAME cpp.convenience_update
  COMMAND belief_update_convenience_update_test
)
set_tests_properties(
  cpp.convenience_update
  PROPERTIES LABELS "unit;equivalence"
)

add_executable(belief_update_shared_vectors_test cpp/shared_vectors_test.cpp)
target_link_libraries(
  belief_update_shared_vectors_test
  PRIVATE belief_update::core
)
target_include_directories(
  belief_update_shared_vectors_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
target_compile_definitions(
  belief_update_shared_vectors_test
  PRIVATE
    "BELIEF_UPDATE_VECTOR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/vectors/bayes_vectors.csv\""
)
belief_update_apply_project_options(belief_update_shared_vectors_test)
add_test(NAME cpp.shared_vectors COMMAND belief_update_shared_vectors_test)
set_tests_properties(
  cpp.shared_vectors
  PROPERTIES LABELS "unit;equivalence;golden;vectors"
)

add_executable(
  belief_update_misleading_posterior_safeguards_test
  cpp/misleading_posterior_safeguards_test.cpp
)
target_link_libraries(
  belief_update_misleading_posterior_safeguards_test
  PRIVATE belief_update::core belief_update::update
)
target_include_directories(
  belief_update_misleading_posterior_safeguards_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(
  belief_update_misleading_posterior_safeguards_test
)
add_test(
  NAME cpp.misleading_posterior_safeguards
  COMMAND belief_update_misleading_posterior_safeguards_test
)
set_tests_properties(
  cpp.misleading_posterior_safeguards
  PROPERTIES LABELS "unit;validation;audit"
)

add_executable(belief_update_stress_test stress/stress_test.cpp)
target_link_libraries(
  belief_update_stress_test
  PRIVATE belief_update::stress belief_update::update
)
target_include_directories(
  belief_update_stress_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_stress_test)
add_test(NAME cpp.stress COMMAND belief_update_stress_test)
set_tests_properties(cpp.stress PROPERTIES LABELS "unit;stress")

add_executable(
  belief_update_local_sensitivity_test
  analysis/local_sensitivity_test.cpp
)
target_link_libraries(
  belief_update_local_sensitivity_test
  PRIVATE belief_update::local_sensitivity belief_update::update
)
target_include_directories(
  belief_update_local_sensitivity_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_local_sensitivity_test)
add_test(
  NAME cpp.local_sensitivity
  COMMAND belief_update_local_sensitivity_test
)
set_tests_properties(
  cpp.local_sensitivity
  PROPERTIES LABELS "unit;analysis"
)

add_executable(
  belief_update_posterior_comparison_test
  analysis/posterior_comparison_test.cpp
)
target_link_libraries(
  belief_update_posterior_comparison_test
  PRIVATE belief_update::posterior_comparison
)
target_include_directories(
  belief_update_posterior_comparison_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_posterior_comparison_test)
add_test(
  NAME cpp.posterior_comparison
  COMMAND belief_update_posterior_comparison_test
)
set_tests_properties(
  cpp.posterior_comparison
  PROPERTIES LABELS "unit;analysis"
)

add_executable(
  belief_update_threshold_analysis_test
  analysis/threshold_analysis_test.cpp
)
target_link_libraries(
  belief_update_threshold_analysis_test
  PRIVATE belief_update::threshold_analysis belief_update::update
)
target_include_directories(
  belief_update_threshold_analysis_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_threshold_analysis_test)
add_test(
  NAME cpp.threshold_analysis
  COMMAND belief_update_threshold_analysis_test
)
set_tests_properties(
  cpp.threshold_analysis
  PROPERTIES LABELS "unit;analysis"
)

add_executable(belief_update_replay_test cpp/replay_test.cpp)
target_link_libraries(
  belief_update_replay_test
  PRIVATE belief_update::replay
)
target_include_directories(
  belief_update_replay_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_replay_test)
add_test(NAME cpp.replay COMMAND belief_update_replay_test)
set_tests_properties(cpp.replay PROPERTIES LABELS "unit;replay")

add_executable(belief_update_tree_test cpp/tree_update_test.cpp)
target_link_libraries(belief_update_tree_test PRIVATE belief_update::tree)
target_include_directories(
  belief_update_tree_test
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cpp"
)
belief_update_apply_project_options(belief_update_tree_test)
add_test(NAME cpp.tree COMMAND belief_update_tree_test)
set_tests_properties(cpp.tree PROPERTIES LABELS "unit;tree;replay")

if(BELIEF_UPDATE_BUILD_PYTHON)
  add_test(
    NAME python.import
    COMMAND
      "${CMAKE_COMMAND}" -E env
      "PYTHONPATH=${PROJECT_BINARY_DIR}/python"
      "${Python_EXECUTABLE}" -m unittest discover
      -s "${CMAKE_CURRENT_SOURCE_DIR}/python"
      -p "test_*.py"
  )
  set_tests_properties(
    python.import
    PROPERTIES
      LABELS "python;unit"
      DEPENDS cpp.smoke
  )
endif()
