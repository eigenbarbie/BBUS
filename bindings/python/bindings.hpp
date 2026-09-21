#pragma once

#include <pybind11/pybind11.h>

namespace belief_update::python {

void bind_analysis(pybind11::module_& module);
void bind_core(pybind11::module_& module);
void bind_model(pybind11::module_& module);
void bind_provenance(pybind11::module_& module);
void bind_replay(pybind11::module_& module);
void bind_resolver(pybind11::module_& module);
void bind_values(pybind11::module_& module);
void bind_result(pybind11::module_& module);
void bind_stress(pybind11::module_& module);
void bind_tree(pybind11::module_& module);
void bind_update(pybind11::module_& module);
void bind_validation(pybind11::module_& module);

}  // namespace belief_update::python
