# Architecture boundary

The production update path is fixed to four stages:

```text
validate -> resolve -> update -> result
```

Future production code will live in narrowly scoped files under `include/` and
`src/`. Input adapters and likelihood models feed validation, but do not become
part of the real-time target. Audit serialization, replay drivers, benchmarks,
fuzzers, and stress harnesses remain separate targets under `tests/` or `tools/`.

The dependency direction is one-way:

```text
Input / Likelihood Model
        |
Validation + Provenance
        |
Deterministic Resolver
        |
Minimal Bayesian Core
        |
Immutable UpdateResult
        |
Audit / Replay / Stress Testing
```

The current repository contains invariant-checked domain value objects, the
minimal binary Bayesian core, an exact categorical hypothesis-tree core,
deterministic validation, and precedence-only binary input resolution. It also
defines a binary likelihood-model interface but contains no production
inference implementation. It does not yet contain input adapters or audit
serialization. Timestamps are always supplied by callers; validation never
reads a clock or introduces an implicit value.

Validation is observational: it returns a typed accept, warn, or reject result
and never changes likelihoods, provenance, or confidence. Resolution refers to
the selected immutable candidate directly. This keeps scalar validation and
selection O(1) and allocation-free on the normal path.

Model inference is outside the Bayesian core. The model and core targets depend
independently on the type layer, so the core receives only resolved scalar
likelihoods and cannot observe their source or optional model metadata.

The tree target is additive and does not change the binary dependency path. It
accepts a validated parent-before-child topology, joint priors for its leaves,
a resolved likelihood for each leaf, and typed provenance:

```text
validate topology/prior/likelihoods/provenance
    -> exact categorical leaf update
    -> aggregate descendant leaves
    -> immutable TreeUpdateResult
```

Tree labels, model inference, serialization, and domain interpretation remain
outside this target. Update cost is O(nodes + leaves). Variable-sized immutable
inputs, intermediates, and results use owned contiguous vectors; no recursive
traversal, virtual dispatch, logging, or formatting occurs in the update.

The convenience update overloads remain orchestration code. They have a fixed
contract—exact binary update and no freshness rule—and then enter the same
`validate -> resolve -> update -> result` path. Direct scalar inputs carry the
typed `direct_input` source kind and deterministic provenance; model inputs
retain the `LikelihoodInput` returned by `LikelihoodModel::infer()`.

Completed updates are self-contained immutable records. `UpdateResult` owns the
original likelihood candidates, prior, evaluation timestamp, configuration,
algorithm choice, resolver decision, intermediate calculation, warning state,
and library version. Resolved values and provenance are views into the owned
selected candidate, preventing duplicated fields from diverging.

Replay is a separate target layered above the normal update pipeline:

```text
recorded UpdateResult -> version check -> validate -> resolve -> update -> result
```

Serialization and formatting may consume an `UpdateResult`, but must remain in
separate targets and must never become dependencies of the core or update path.

Stress analysis is another one-way consumer of `UpdateResult`:

```text
immutable UpdateResult + explicit ranges -> core corner updates -> StressResult
```

The `belief_update::stress` target depends only on the type and core targets.
Neither the core nor production update target depends on stress analysis. All
three probability ranges are mandatory; callers choose either relative
percentages or absolute bounds. Invalid or out-of-range bounds fail rather than
being clipped, and the original update record is observed through a const
reference.

Optional analysis follows the same one-way dependency rule:

```text
UpdateResult -> local sensitivity
UpdateResult -> model/explicit comparison
UpdateResult -> threshold analysis
```

Each concern has its own link target and focused source file. Sensitivity uses
closed-form partial derivatives, comparison reuses the recorded update pipeline,
and threshold analysis uses a closed-form threshold likelihood ratio. These
targets do not introduce autodiff or optimization libraries and are not linked
into the real-time path or Python extension.
