#pragma once

#include <span>
#include <vector>

namespace belief_update {

class TreeLikelihoodValues final {
 public:
  // Values correspond to HypothesisTree::leaf_ids() in ascending node order.
  explicit TreeLikelihoodValues(std::vector<double> evidence_given_leaves);

  TreeLikelihoodValues(const TreeLikelihoodValues&) = default;
  TreeLikelihoodValues(TreeLikelihoodValues&&) noexcept = default;
  TreeLikelihoodValues& operator=(const TreeLikelihoodValues&) = delete;
  TreeLikelihoodValues& operator=(TreeLikelihoodValues&&) = delete;

  [[nodiscard]] std::span<const double> evidence_given_leaves()
      const noexcept {
    return evidence_given_leaves_;
  }

 private:
  std::vector<double> evidence_given_leaves_;
};

}  // namespace belief_update
