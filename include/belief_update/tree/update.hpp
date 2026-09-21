#pragma once

#include "belief_update/tree/tree_belief_state.hpp"
#include "belief_update/tree/tree_likelihood_values.hpp"
#include "belief_update/tree/tree_update_result.hpp"
#include "belief_update/types/provenance.hpp"

namespace belief_update {

// Performs exact categorical Bayes over the leaves. Internal node
// probabilities are deterministic sums of descendant leaf posteriors.
[[nodiscard]] TreeUpdateResult update_tree(
    const TreeBeliefState& prior, const TreeLikelihoodValues& likelihoods,
    const Provenance& provenance);

}  // namespace belief_update
