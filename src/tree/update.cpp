#include "belief_update/tree/update.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "belief_update/types/update_config.hpp"
#include "belief_update/validation/validator.hpp"
#include "belief_update/version.hpp"

namespace belief_update {

TreeUpdateResult update_tree(const TreeBeliefState& prior,
                             const TreeLikelihoodValues& likelihoods,
                             const Provenance& provenance) {
  const Validator validator{UpdateConfig{std::nullopt}};
  const ValidationResult provenance_validation =
      validator.validate_provenance(provenance, provenance.timestamp());
  if (!provenance_validation.can_proceed()) {
    throw std::invalid_argument("tree update rejected: malformed provenance");
  }

  const std::span<const double> prior_leaves = prior.leaf_probabilities();
  const std::span<const double> likelihood_values =
      likelihoods.evidence_given_leaves();
  if (prior_leaves.size() != likelihood_values.size()) {
    throw std::invalid_argument(
        "tree likelihood count must match the tree leaf count");
  }

  const double likelihood_scale =
      *std::max_element(likelihood_values.begin(), likelihood_values.end());
  if (likelihood_scale == 0.0) {
    throw std::domain_error(
        "evidence has zero likelihood under every leaf possibility");
  }

  std::vector<double> scaled_leaf_masses;
  scaled_leaf_masses.reserve(prior_leaves.size());
  double scaled_evidence_mass = 0.0;
  for (std::size_t index = 0; index < prior_leaves.size(); ++index) {
    const double mass =
        prior_leaves[index] * (likelihood_values[index] / likelihood_scale);
    scaled_leaf_masses.push_back(mass);
    scaled_evidence_mass += mass;
  }
  if (scaled_evidence_mass == 0.0) {
    throw std::domain_error("evidence has zero probability under the prior");
  }

  std::vector<double> posterior_leaves;
  posterior_leaves.reserve(scaled_leaf_masses.size());
  for (const double mass : scaled_leaf_masses) {
    posterior_leaves.push_back(mass / scaled_evidence_mass);
  }
  TreeBeliefState posterior{prior.tree(), std::move(posterior_leaves),
                            prior.probability_sum_tolerance(),
                            TreeBeliefState::PosteriorTag{}};

  return TreeUpdateResult{prior,
                          likelihoods,
                          provenance,
                          likelihood_scale,
                          std::move(scaled_leaf_masses),
                          scaled_evidence_mass,
                          std::move(posterior),
                          std::string{version}};
}

}  // namespace belief_update
