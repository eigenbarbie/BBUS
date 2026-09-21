#pragma once

#include <span>
#include <vector>

#include "belief_update/tree/hypothesis_tree.hpp"

namespace belief_update {

class TreeLikelihoodValues;
class TreeUpdateResult;
class Provenance;

class TreeBeliefState final {
 public:
  // Leaf probabilities correspond to tree.leaf_ids() in ascending node order.
  // Values are preserved exactly; they are never silently normalized.
  explicit TreeBeliefState(HypothesisTree tree,
                           std::vector<double> leaf_probabilities,
                           double probability_sum_tolerance);

  TreeBeliefState(const TreeBeliefState&) = default;
  TreeBeliefState(TreeBeliefState&&) noexcept = default;
  TreeBeliefState& operator=(const TreeBeliefState&) = delete;
  TreeBeliefState& operator=(TreeBeliefState&&) = delete;

  [[nodiscard]] const HypothesisTree& tree() const noexcept { return tree_; }
  [[nodiscard]] std::span<const double> leaf_probabilities() const noexcept {
    return leaf_probabilities_;
  }
  [[nodiscard]] std::span<const double> node_probabilities() const noexcept {
    return node_probabilities_;
  }
  [[nodiscard]] double probability(HypothesisId node) const;
  [[nodiscard]] double probability_sum_tolerance() const noexcept {
    return probability_sum_tolerance_;
  }

 private:
  struct PosteriorTag final {};

  friend TreeUpdateResult update_tree(const TreeBeliefState&,
                                      const TreeLikelihoodValues&,
                                      const Provenance&);

  explicit TreeBeliefState(HypothesisTree tree,
                           std::vector<double> leaf_probabilities,
                           double probability_sum_tolerance, PosteriorTag);

  HypothesisTree tree_;
  std::vector<double> leaf_probabilities_;
  std::vector<double> node_probabilities_;
  double probability_sum_tolerance_;
};

}  // namespace belief_update
