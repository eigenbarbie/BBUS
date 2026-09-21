#include "belief_update/tree/hypothesis_tree.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "belief_update/detail/probability.hpp"
#include "belief_update/tree/tree_belief_state.hpp"
#include "belief_update/tree/tree_likelihood_values.hpp"

namespace belief_update {
namespace {

[[nodiscard]] std::vector<double> aggregate_node_probabilities(
    const HypothesisTree& tree, const std::vector<double>& leaf_probabilities) {
  std::vector<double> node_probabilities(tree.node_count(), 0.0);
  const std::span<const HypothesisId> leaves = tree.leaf_ids();
  for (std::size_t index = 0; index < leaves.size(); ++index) {
    node_probabilities[leaves[index].value()] = leaf_probabilities[index];
  }
  for (std::size_t index = tree.node_count(); index-- > 1;) {
    const HypothesisId parent = *tree.parent(
        HypothesisId{static_cast<std::uint32_t>(index)});
    node_probabilities[parent.value()] += node_probabilities[index];
  }
  return node_probabilities;
}

[[nodiscard]] double ordered_sum(const std::vector<double>& values) noexcept {
  double sum = 0.0;
  double compensation = 0.0;
  for (const double value : values) {
    const double corrected = value - compensation;
    const double updated = sum + corrected;
    compensation = (updated - sum) - corrected;
    sum = updated;
  }
  return sum;
}

void validate_tolerance(double tolerance) {
  if (!std::isfinite(tolerance) || tolerance < 0.0 || tolerance >= 1.0) {
    throw std::invalid_argument(
        "probability sum tolerance must be finite and in [0, 1)");
  }
}

void validate_leaf_probabilities(const HypothesisTree& tree,
                                 const std::vector<double>& probabilities,
                                 double tolerance) {
  if (probabilities.size() != tree.leaf_ids().size()) {
    throw std::invalid_argument(
        "leaf probability count must match the tree leaf count");
  }
  for (const double probability : probabilities) {
    static_cast<void>(detail::checked_probability(probability));
  }
  if (std::abs(ordered_sum(probabilities) - 1.0) > tolerance) {
    throw std::invalid_argument(
        "leaf probabilities must sum to one within the supplied tolerance");
  }
}

}  // namespace

HypothesisTree::HypothesisTree(
    std::vector<std::optional<HypothesisId>> parents)
    : parents_(std::move(parents)) {
  if (parents_.size() >
      static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
    throw std::invalid_argument("hypothesis tree has too many nodes");
  }
  if (parents_.empty() || parents_.front().has_value()) {
    throw std::invalid_argument("hypothesis tree node zero must be the root");
  }

  std::vector<std::size_t> child_counts(parents_.size(), 0);
  for (std::size_t index = 1; index < parents_.size(); ++index) {
    if (!parents_[index].has_value()) {
      throw std::invalid_argument(
          "every non-root hypothesis node requires a parent");
    }
    const std::size_t parent_index = parents_[index]->value();
    if (parent_index >= index) {
      throw std::invalid_argument(
          "a hypothesis parent must precede its child");
    }
    ++child_counts[parent_index];
  }

  for (std::size_t index = 0; index < child_counts.size(); ++index) {
    if (child_counts[index] == 0) {
      leaf_ids_.emplace_back(static_cast<std::uint32_t>(index));
    }
  }
  if (leaf_ids_.size() < 2) {
    throw std::invalid_argument(
        "a hypothesis tree requires at least two leaf possibilities");
  }
}

std::size_t HypothesisTree::checked_index(HypothesisId node) const {
  const std::size_t index = node.value();
  if (index >= parents_.size()) {
    throw std::out_of_range("hypothesis node id is outside the tree");
  }
  return index;
}

const std::optional<HypothesisId>& HypothesisTree::parent(
    HypothesisId node) const {
  return parents_[checked_index(node)];
}

bool HypothesisTree::is_leaf(HypothesisId node) const {
  const std::size_t index = checked_index(node);
  for (std::size_t candidate = index + 1; candidate < parents_.size();
       ++candidate) {
    if (parents_[candidate]->value() == index) {
      return false;
    }
  }
  return true;
}

TreeBeliefState::TreeBeliefState(HypothesisTree tree,
                                 std::vector<double> leaf_probabilities,
                                 double probability_sum_tolerance)
    : tree_(std::move(tree)),
      leaf_probabilities_(std::move(leaf_probabilities)),
      probability_sum_tolerance_(probability_sum_tolerance) {
  validate_tolerance(probability_sum_tolerance_);
  validate_leaf_probabilities(tree_, leaf_probabilities_,
                              probability_sum_tolerance_);
  node_probabilities_ =
      aggregate_node_probabilities(tree_, leaf_probabilities_);
}

TreeBeliefState::TreeBeliefState(HypothesisTree tree,
                                 std::vector<double> leaf_probabilities,
                                 double probability_sum_tolerance,
                                 PosteriorTag)
    : tree_(std::move(tree)),
      leaf_probabilities_(std::move(leaf_probabilities)),
      node_probabilities_(
          aggregate_node_probabilities(tree_, leaf_probabilities_)),
      probability_sum_tolerance_(probability_sum_tolerance) {}

double TreeBeliefState::probability(HypothesisId node) const {
  return node_probabilities_.at(node.value());
}

TreeLikelihoodValues::TreeLikelihoodValues(
    std::vector<double> evidence_given_leaves)
    : evidence_given_leaves_(std::move(evidence_given_leaves)) {
  if (evidence_given_leaves_.empty()) {
    throw std::invalid_argument("tree likelihood values must not be empty");
  }
  for (const double probability : evidence_given_leaves_) {
    static_cast<void>(detail::checked_probability(probability));
  }
}

}  // namespace belief_update
