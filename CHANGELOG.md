# Changelog

BBUS follows semantic versioning in `MAJOR.MINOR.PATCH` form. Git release tags
use the matching `vMAJOR.MINOR.PATCH` form.

## 0.1.0

- Added exact categorical Bayesian updates over an explicit hypothesis tree.
- Added typed node IDs, immutable tree priors, leaf likelihoods, complete tree
  results, and deterministic tree replay.
- Added thin Python bindings and matching C++/Python correctness tests.
- Extended both demos and benchmarks with a nested three-leaf tree example.
- Preserved the existing binary update API without behavioral changes.

## 0.0.3

- First public repository release of the deterministic binary update pipeline.
- Included validation, deterministic resolution, immutable audit results,
  replay, stress analysis, optional analysis modules, and Python bindings.
