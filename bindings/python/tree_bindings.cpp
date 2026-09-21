#include "bindings.hpp"

#include <cstddef>
#include <optional>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "belief_update/tree.hpp"

namespace py = pybind11;

namespace belief_update::python {
namespace {

[[nodiscard]] py::tuple parent_tuple(const HypothesisTree& tree) {
  const auto parents = tree.parents();
  py::tuple result(parents.size());
  for (std::size_t index = 0; index < parents.size(); ++index) {
    result[index] = parents[index].has_value() ? py::cast(*parents[index])
                                               : py::none();
  }
  return result;
}

[[nodiscard]] py::tuple leaf_id_tuple(const HypothesisTree& tree) {
  const auto leaves = tree.leaf_ids();
  py::tuple result(leaves.size());
  for (std::size_t index = 0; index < leaves.size(); ++index) {
    result[index] = py::cast(leaves[index]);
  }
  return result;
}

[[nodiscard]] py::tuple double_tuple(std::span<const double> values) {
  py::tuple result(values.size());
  for (std::size_t index = 0; index < values.size(); ++index) {
    result[index] = values[index];
  }
  return result;
}

}  // namespace

void bind_tree(py::module_& module) {
  py::class_<HypothesisId>(module, "HypothesisId")
      .def(py::init<std::uint32_t>(), py::arg("value"))
      .def_property_readonly("value", &HypothesisId::value)
      .def("__eq__",
           [](HypothesisId left, HypothesisId right) { return left == right; },
           py::is_operator());

  py::class_<HypothesisTree>(module, "HypothesisTree")
      .def(py::init<std::vector<std::optional<HypothesisId>>>(),
           py::arg("parents"))
      .def_property_readonly("node_count", &HypothesisTree::node_count)
      .def_property_readonly("parents", &parent_tuple)
      .def("parent", &HypothesisTree::parent, py::arg("node"))
      .def_property_readonly("leaf_ids", &leaf_id_tuple)
      .def("is_leaf", &HypothesisTree::is_leaf, py::arg("node"));

  py::class_<TreeBeliefState>(module, "TreeBeliefState")
      .def(py::init<HypothesisTree, std::vector<double>, double>(),
           py::arg("tree"), py::arg("leaf_probabilities"),
           py::arg("probability_sum_tolerance"))
      .def_property_readonly("tree", &TreeBeliefState::tree)
      .def_property_readonly(
          "leaf_probabilities",
          [](const TreeBeliefState& state) {
            return double_tuple(state.leaf_probabilities());
          })
      .def_property_readonly(
          "node_probabilities",
          [](const TreeBeliefState& state) {
            return double_tuple(state.node_probabilities());
          })
      .def("probability", &TreeBeliefState::probability, py::arg("node"))
      .def_property_readonly("probability_sum_tolerance",
                             &TreeBeliefState::probability_sum_tolerance);

  py::class_<TreeLikelihoodValues>(module, "TreeLikelihoodValues")
      .def(py::init<std::vector<double>>(),
           py::arg("evidence_given_leaves"))
      .def_property_readonly(
          "evidence_given_leaves",
          [](const TreeLikelihoodValues& likelihoods) {
            return double_tuple(likelihoods.evidence_given_leaves());
          });

  py::enum_<TreeUpdateAlgorithm>(module, "TreeUpdateAlgorithm")
      .value("exact_categorical", TreeUpdateAlgorithm::exact_categorical);

  py::class_<TreeUpdateResult>(module, "TreeUpdateResult")
      .def_property_readonly("prior", &TreeUpdateResult::prior)
      .def_property_readonly("likelihoods", &TreeUpdateResult::likelihoods)
      .def_property_readonly("provenance", &TreeUpdateResult::provenance)
      .def_property_readonly("algorithm", &TreeUpdateResult::algorithm)
      .def_property_readonly("likelihood_scale",
                             &TreeUpdateResult::likelihood_scale)
      .def_property_readonly(
          "scaled_leaf_masses",
          [](const TreeUpdateResult& result) {
            return double_tuple(result.scaled_leaf_masses());
          })
      .def_property_readonly("scaled_evidence_mass",
                             &TreeUpdateResult::scaled_evidence_mass)
      .def_property_readonly("posterior", &TreeUpdateResult::posterior)
      .def_property_readonly("library_version",
                             &TreeUpdateResult::library_version);

  module.def("update_tree", &update_tree, py::arg("prior"),
             py::arg("likelihoods"), py::arg("provenance"));
  module.def(
      "replay",
      static_cast<TreeUpdateResult (*)(const TreeUpdateResult&)>(&replay),
      py::arg("recorded"));
}

}  // namespace belief_update::python
