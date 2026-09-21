#include <pybind11/pybind11.h>

#include "belief_update/version.hpp"
#include "bindings.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_belief_update, module) {
  module.doc() = "Deterministic Bayesian belief-update primitives";
  module.attr("__version__") = belief_update::version;

  belief_update::python::bind_provenance(module);
  belief_update::python::bind_values(module);
  belief_update::python::bind_result(module);
  belief_update::python::bind_core(module);
  belief_update::python::bind_validation(module);
  belief_update::python::bind_resolver(module);
  belief_update::python::bind_model(module);
  belief_update::python::bind_update(module);
  belief_update::python::bind_analysis(module);
  belief_update::python::bind_stress(module);
  belief_update::python::bind_tree(module);
  belief_update::python::bind_replay(module);
}
