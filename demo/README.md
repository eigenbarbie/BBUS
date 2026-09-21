# BBUS capability demo and benchmark

The two demo programs exercise the same library capabilities from C++ and
Python:

- typed provenance and explicit-over-model input precedence;
- exact binary and log-odds updates;
- exact categorical updates over a nested hypothesis tree;
- immutable results and deterministic replay;
- local sensitivity and model-versus-explicit comparison;
- threshold analysis; and
- caller-supplied stress ranges without modifying the original result.

They also time the mathematical core and the complete
`validate -> resolve -> update -> result` pipeline. The Python runner creates a
CSV table, a Markdown report, and an SVG chart using only the Python standard
library.

## What the comparison means

scikit-learn has no direct equivalent of BBUS's audited belief-update record.
The optional comparison therefore configures `BernoulliNB` with counts that
encode the same prior and likelihoods, then measures one
`predict_proba` call for one observation. It is a useful general-library
reference point, but it does not validate provenance or provide replay and
stress-analysis semantics.

The unchecked C++ and Python formulas are latency floors. They deliberately
omit validation, numerical edge handling, typed provenance, resolution, and an
immutable result. They are not replacement implementations.

Benchmarks are medians of seven trials. Run a Release build on an otherwise
idle machine; results vary with hardware, compiler, Python, power mode, and
background load.

## Example results

![BBUS latency comparison](results/benchmark_results.svg)

The chart uses a **logarithmic time scale**: every vertical grid line represents
10 times as much latency. `1 us` (one microsecond) is `1,000 ns` (nanoseconds),
and `1 ms` (one millisecond) is `1,000 us`. Lower is better.

| Implementation | Median time for one update |
|---|---:|
| BBUS C++ core | 5.8 ns |
| BBUS C++ audited update | 33.9 ns |
| BBUS C++ three-leaf tree update | 288.6 ns |
| BBUS Python core binding | 528.6 ns |
| BBUS Python audited update | 725.2 ns |
| BBUS Python three-leaf tree update | 1.31 us |
| scikit-learn `BernoulliNB.predict_proba` | 131.47 us |

These example measurements were produced on arm64 macOS using an Apple Clang
17 Release build, Python 3.14.1, BBUS 0.1.0, and scikit-learn 1.9.1. See the
[full generated report](results/benchmark_results.md) for the unchecked latency
floors, iteration counts, environment details, and comparison limitations.

## Environment setup

These commands are for macOS or Linux from the repository root. Requirements:

- CMake 3.25 or newer;
- a C++20 compiler (Apple Clang, Clang, or GCC); and
- Python 3.9 or newer.

Create an isolated Python environment and install BBUS:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install .
```

Install scikit-learn only if you want its optional comparison:

```sh
python -m pip install scikit-learn
```

This does not add scikit-learn, NumPy, SciPy, or plotting packages to BBUS's
runtime dependencies.

## Build and run the C++ demo

Configure a Release build without Python or tests, enable the demo, then build
its target:

```sh
cmake -S . -B build/demo \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DBELIEF_UPDATE_BUILD_PYTHON=OFF \
  -DBELIEF_UPDATE_BUILD_DEMOS=ON
cmake --build build/demo --target belief_update_demo --config Release
./build/demo/demo/belief_update_demo 200000
```

The optional integer argument is the number of updates per timing trial. If it
is omitted, the C++ demo uses `200000`.

## Run the Python demo and create the plot

With the virtual environment still active:

```sh
python demo/belief_update_demo.py \
  --cpp-executable build/demo/demo/belief_update_demo \
  --iterations 10000 \
  --cpp-iterations 200000 \
  --output-dir demo/results
```

The command writes:

- `demo/results/benchmark_results.csv` — raw median timings;
- `demo/results/benchmark_results.md` — a readable table and environment; and
- `demo/results/benchmark_results.svg` — a logarithmic latency chart.

Use `--skip-sklearn` when that optional dependency is not installed. Omit
`--cpp-executable` to run only the Python-side measurements.

For less noisy results, increase both iteration counts. Do not compare results
from different machines or build modes as if they were equivalent.
