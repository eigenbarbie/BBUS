# Demo benchmark results

Median of seven trials. Lower latency is better.

| Implementation | Median per update | Iterations/trial | Posterior | Scope |
|---|---:|---:|---:|---|
| `cpp_unchecked_formula` | 0.9 ns | 200,000 | 0.727272727 | unchecked arithmetic latency floor |
| `bbus_cpp_core` | 5.9 ns | 200,000 | 0.727272727 | validated C++ mathematical core |
| `bbus_cpp_audited_update` | 33.9 ns | 200,000 | 0.727272727 | complete C++ validate-resolve-update-result path |
| `python_unchecked_formula` | 154.6 ns | 10,000 | 0.727272727 | unchecked Python arithmetic latency floor |
| `bbus_cpp_tree_update` | 288.6 ns | 200,000 | 0.848484848 | complete three-leaf categorical tree update |
| `bbus_python_core_binding` | 528.6 ns | 10,000 | 0.727272727 | Python-to-C++ mathematical core binding |
| `bbus_python_audited_update` | 725.2 ns | 10,000 | 0.727272727 | Python-to-C++ complete validate-resolve-update-result path |
| `bbus_python_tree_update` | 1.31 us | 10,000 | 0.848484848 | Python-to-C++ complete three-leaf categorical tree update |
| `sklearn_bernoulli_nb_scalar` | 131.47 us | 10,000 | 0.727272727 | scalar BernoulliNB.predict_proba on equivalent evidence |

- Platform: `macOS-26.5.2-arm64-arm-64bit-Mach-O`
- Python: `3.14.1`
- BBUS: `0.1.0`
- scikit-learn: `1.9.1`

These are illustrative local measurements, not universal performance claims. The scikit-learn row measures scalar `BernoulliNB.predict_proba`; it does not provide BBUS provenance, validation, replay, or stress-analysis semantics. Unchecked formula rows omit safety and audit behavior.
