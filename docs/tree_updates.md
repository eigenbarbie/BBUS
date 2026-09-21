# Exact categorical hypothesis trees

The tree API represents a mutually exclusive and collectively exhaustive set of
leaf possibilities under a hierarchy. It is not a Bayesian network and does
not model conditional dependencies between arbitrary nodes.

## Topology

`HypothesisId` is a typed unsigned node index. `HypothesisTree` requires:

- node zero is the single root;
- every other node has exactly one parent;
- every parent has a smaller ID than its child;
- all nodes are therefore connected and acyclic; and
- at least two leaves exist.

The parent-before-child rule makes validation and aggregation iterative and
deterministic. Human-readable labels belong in the calling application and do
not enter the mathematical update path.

## Inputs

`TreeBeliefState` receives joint prior probabilities for the leaves in the
order returned by `HypothesisTree::leaf_ids()`. Every value must be finite and
in `[0, 1]`. The caller supplies `probability_sum_tolerance` explicitly, and the
leaf priors must sum to one within that tolerance. BBUS records the original
values and never normalizes or clips them.

`TreeLikelihoodValues` supplies `P(E | leaf)` in the same leaf order. The count
must match the tree exactly. Typed `Provenance` describes the source of that
complete likelihood vector.

## Update

For each leaf `i`, exact categorical Bayes is:

```text
mass[i] = prior[i] * P(E | leaf[i])
posterior[i] = mass[i] / sum(mass)
```

The implementation first divides likelihoods by their maximum value. This
common scale cancels during normalization and avoids avoidable underflow for
very small likelihoods. `TreeUpdateResult` records the likelihood scale, scaled
leaf masses, and scaled evidence mass so the calculation is auditable.

After the leaf update, internal-node posterior probabilities are calculated by
summing descendant leaves in reverse node order. No evidence is attached
directly to internal nodes.

The update rejects:

- malformed topology or provenance;
- invalid or incorrectly sized probability vectors;
- priors outside the caller-supplied sum tolerance;
- zero likelihood under every leaf; and
- evidence with zero probability under the supplied prior.

## Result and replay

`TreeUpdateResult` is immutable after creation and owns everything needed for
replay: prior tree state, likelihood vector, provenance, algorithm,
intermediates, posterior tree state, and library version.

```text
replay(TreeUpdateResult) -> reproduced TreeUpdateResult
```

Replay requires the recorded library version to match the running library and
then calls the same production tree update function.

## Deliberate limits in v0.1.0

- Existing binary APIs remain unchanged.
- Tree likelihood-model adapters are caller-owned.
- Tree stress ranges and sensitivity analysis are not inferred automatically.
- Sequential dependent evidence is not made independent by the hierarchy.
- The API does not perform belief propagation over a general graph.
