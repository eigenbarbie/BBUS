include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_file(
  "${PROJECT_SOURCE_DIR}/include/belief_update/version.hpp.in"
  "${PROJECT_BINARY_DIR}/generated/belief_update/version.hpp"
  @ONLY
)

add_library(belief_update_types INTERFACE)
add_library(belief_update::types ALIAS belief_update_types)
set_target_properties(belief_update_types PROPERTIES EXPORT_NAME types)
target_compile_features(belief_update_types INTERFACE cxx_std_20)
target_include_directories(
  belief_update_types
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_library(belief_update_model INTERFACE)
add_library(belief_update::model ALIAS belief_update_model)
set_target_properties(belief_update_model PROPERTIES EXPORT_NAME model)
target_link_libraries(belief_update_model INTERFACE belief_update::types)

add_library(belief_update_core STATIC core/bayes.cpp)
add_library(belief_update::core ALIAS belief_update_core)
set_target_properties(
  belief_update_core
  PROPERTIES
    EXPORT_NAME core
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(belief_update_core PUBLIC belief_update::types)
belief_update_apply_project_options(belief_update_core)

add_library(
  belief_update_tree
  STATIC
    tree/types.cpp
    tree/update.cpp
    tree/replay.cpp
)
add_library(belief_update::tree ALIAS belief_update_tree)
set_target_properties(
  belief_update_tree
  PROPERTIES
    EXPORT_NAME tree
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(belief_update_tree PUBLIC belief_update::types)
belief_update_apply_project_options(belief_update_tree)

add_library(belief_update_validation STATIC validation/validator.cpp)
add_library(belief_update::validation ALIAS belief_update_validation)
set_target_properties(
  belief_update_validation
  PROPERTIES
    EXPORT_NAME validation
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(belief_update_validation PUBLIC belief_update::types)
belief_update_apply_project_options(belief_update_validation)
target_link_libraries(belief_update_tree PUBLIC belief_update::validation)

add_library(belief_update_resolver STATIC resolver/input_resolver.cpp)
add_library(belief_update::resolver ALIAS belief_update_resolver)
set_target_properties(
  belief_update_resolver
  PROPERTIES
    EXPORT_NAME resolver
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_resolver
  PUBLIC belief_update::types belief_update::validation
)
belief_update_apply_project_options(belief_update_resolver)

add_library(belief_update_update STATIC update/update.cpp)
add_library(belief_update::update ALIAS belief_update_update)
set_target_properties(
  belief_update_update
  PROPERTIES
    EXPORT_NAME update
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_update
  PUBLIC
    belief_update::types
    belief_update::core
    belief_update::resolver
  PRIVATE belief_update::model
)
belief_update_apply_project_options(belief_update_update)

add_library(belief_update_replay STATIC replay/replay.cpp)
add_library(belief_update::replay ALIAS belief_update_replay)
set_target_properties(
  belief_update_replay
  PROPERTIES
    EXPORT_NAME replay
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_replay
  PUBLIC belief_update::types belief_update::update
)
belief_update_apply_project_options(belief_update_replay)

add_library(belief_update_stress STATIC stress/stress.cpp)
add_library(belief_update::stress ALIAS belief_update_stress)
set_target_properties(
  belief_update_stress
  PROPERTIES
    EXPORT_NAME stress
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_stress
  PUBLIC belief_update::types belief_update::core
)
belief_update_apply_project_options(belief_update_stress)

add_library(
  belief_update_local_sensitivity
  STATIC analysis/local_sensitivity.cpp
)
add_library(
  belief_update::local_sensitivity
  ALIAS belief_update_local_sensitivity
)
set_target_properties(
  belief_update_local_sensitivity
  PROPERTIES
    EXPORT_NAME local_sensitivity
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_local_sensitivity
  PUBLIC belief_update::types
)
belief_update_apply_project_options(belief_update_local_sensitivity)

add_library(
  belief_update_posterior_comparison
  STATIC analysis/posterior_comparison.cpp
)
add_library(
  belief_update::posterior_comparison
  ALIAS belief_update_posterior_comparison
)
set_target_properties(
  belief_update_posterior_comparison
  PROPERTIES
    EXPORT_NAME posterior_comparison
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_posterior_comparison
  PUBLIC belief_update::types belief_update::update
)
belief_update_apply_project_options(belief_update_posterior_comparison)

add_library(
  belief_update_threshold_analysis
  STATIC analysis/threshold_analysis.cpp
)
add_library(
  belief_update::threshold_analysis
  ALIAS belief_update_threshold_analysis
)
set_target_properties(
  belief_update_threshold_analysis
  PROPERTIES
    EXPORT_NAME threshold_analysis
    POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
  belief_update_threshold_analysis
  PUBLIC belief_update::types
)
belief_update_apply_project_options(belief_update_threshold_analysis)

install(
  TARGETS
    belief_update_types
    belief_update_model
    belief_update_core
    belief_update_tree
    belief_update_validation
    belief_update_resolver
    belief_update_update
    belief_update_replay
    belief_update_stress
    belief_update_local_sensitivity
    belief_update_posterior_comparison
    belief_update_threshold_analysis
  EXPORT belief_updateTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
install(
  FILES "${PROJECT_BINARY_DIR}/generated/belief_update/version.hpp"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/belief_update"
)
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include/belief_update/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/belief_update"
  FILES_MATCHING PATTERN "*.hpp"
)
install(
  EXPORT belief_updateTargets
  NAMESPACE belief_update::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/belief_update"
)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/belief_updateConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/belief_updateConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/belief_update"
)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/belief_updateConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${PROJECT_BINARY_DIR}/belief_updateConfig.cmake"
    "${PROJECT_BINARY_DIR}/belief_updateConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/belief_update"
)
