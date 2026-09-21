#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "belief_update/tree/tree_belief_state.hpp"
#include "belief_update/tree/tree_likelihood_values.hpp"
#include "belief_update/types/provenance.hpp"

namespace belief_update {

enum class TreeUpdateAlgorithm : std::uint8_t {
  exact_categorical,
};

class TreeUpdateResult final {
 public:
  TreeUpdateResult(const TreeUpdateResult&) = default;
  TreeUpdateResult(TreeUpdateResult&&) noexcept = default;
  TreeUpdateResult& operator=(const TreeUpdateResult&) = delete;
  TreeUpdateResult& operator=(TreeUpdateResult&&) = delete;

  [[nodiscard]] const TreeBeliefState& prior() const noexcept { return prior_; }
  [[nodiscard]] const TreeLikelihoodValues& likelihoods() const noexcept {
    return likelihoods_;
  }
  [[nodiscard]] const Provenance& provenance() const noexcept {
    return provenance_;
  }
  [[nodiscard]] TreeUpdateAlgorithm algorithm() const noexcept {
    return TreeUpdateAlgorithm::exact_categorical;
  }
  [[nodiscard]] double likelihood_scale() const noexcept {
    return likelihood_scale_;
  }
  [[nodiscard]] std::span<const double> scaled_leaf_masses() const noexcept {
    return scaled_leaf_masses_;
  }
  [[nodiscard]] double scaled_evidence_mass() const noexcept {
    return scaled_evidence_mass_;
  }
  [[nodiscard]] const TreeBeliefState& posterior() const noexcept {
    return posterior_;
  }
  [[nodiscard]] const std::string& library_version() const noexcept {
    return library_version_;
  }

 private:
  friend TreeUpdateResult update_tree(const TreeBeliefState&,
                                      const TreeLikelihoodValues&,
                                      const Provenance&);

  explicit TreeUpdateResult(TreeBeliefState prior,
                            TreeLikelihoodValues likelihoods,
                            Provenance provenance, double likelihood_scale,
                            std::vector<double> scaled_leaf_masses,
                            double scaled_evidence_mass,
                            TreeBeliefState posterior,
                            std::string library_version)
      : prior_(std::move(prior)),
        likelihoods_(std::move(likelihoods)),
        provenance_(std::move(provenance)),
        likelihood_scale_(likelihood_scale),
        scaled_leaf_masses_(std::move(scaled_leaf_masses)),
        scaled_evidence_mass_(scaled_evidence_mass),
        posterior_(std::move(posterior)),
        library_version_(std::move(library_version)) {}

  TreeBeliefState prior_;
  TreeLikelihoodValues likelihoods_;
  Provenance provenance_;
  double likelihood_scale_;
  std::vector<double> scaled_leaf_masses_;
  double scaled_evidence_mass_;
  TreeBeliefState posterior_;
  std::string library_version_;
};

}  // namespace belief_update
