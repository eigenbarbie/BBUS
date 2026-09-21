#!/usr/bin/env python3
"""Demonstrate BBUS and generate reproducible latency comparison artifacts."""

from __future__ import annotations

import argparse
import csv
import html
import math
from pathlib import Path
import platform
import statistics
import subprocess
import sys
import time
from typing import Callable, NamedTuple

import belief_update


class Benchmark(NamedTuple):
    name: str
    median_ns: float
    iterations: int
    posterior: float
    scope: str


def make_input(
    p_e_given_h: float,
    p_e_given_not_h: float,
    source_type: belief_update.SourceType,
    source_id: str,
    override_status: belief_update.OverrideStatus,
) -> belief_update.LikelihoodInput:
    return belief_update.LikelihoodInput(
        likelihoods=belief_update.LikelihoodValues(
            evidence_given_hypothesis=p_e_given_h,
            evidence_given_not_hypothesis=p_e_given_not_h,
        ),
        evidence_weight=None,
        provenance=belief_update.Provenance(
            source_type=source_type,
            source_id=belief_update.SourceId(value=source_id),
            version=belief_update.SourceVersion(value="demo-v1"),
            timestamp=belief_update.ProvenanceTimestamp(
                unix_nanoseconds=1_000
            ),
            confidence=0.95,
            override_status=override_status,
        ),
    )


def showcase() -> None:
    explicit_input = make_input(
        0.8,
        0.3,
        belief_update.SourceType.human,
        "analyst-7",
        belief_update.OverrideStatus.requested,
    )
    model_input = make_input(
        0.8,
        0.2,
        belief_update.SourceType.model,
        "model-a",
        belief_update.OverrideStatus.not_requested,
    )
    result = belief_update.update(
        inputs=belief_update.UpdateInputs(
            prior=belief_update.BeliefState(probability=0.4),
            explicit_override=explicit_input,
            likelihood_model_value=model_input,
            evaluation_time=belief_update.ProvenanceTimestamp(
                unix_nanoseconds=1_000
            ),
        ),
        configuration=belief_update.UpdateConfig(freshness_rule=None),
        algorithm=belief_update.UpdateAlgorithm.exact_binary,
    )
    calculation = result.intermediate_calculations
    replayed = belief_update.replay(result)
    sensitivity = belief_update.local_sensitivity(result=result)
    comparison = belief_update.compare_model_and_explicit(result=result)
    threshold = belief_update.analyze_threshold(result=result, threshold=0.6)
    stress = belief_update.stress_update(
        result=result,
        ranges=belief_update.RelativeStressRanges(
            prior_percent=10.0,
            p_e_given_h_percent=10.0,
            p_e_given_not_h_percent=10.0,
        ),
        threshold=0.7,
    )
    log_odds = belief_update.log_odds_update(
        prior=belief_update.BeliefState(probability=0.4),
        log_likelihood_ratio=math.log(0.8 / 0.2),
    )

    print("BBUS Python capability demo")
    print(f"  prior:                    {calculation.prior:.9f}")
    print(f"  likelihood ratio:         {calculation.likelihood_ratio:.9f}")
    print(f"  posterior:                {result.posterior:.9f}")
    print(f"  belief delta:             {calculation.belief_delta:.9f}")
    print(f"  provenance source id:     {result.provenance.source_id.value}")
    print(f"  replay identical:         {replayed.posterior == result.posterior}")
    print(f"  log-odds posterior:       {log_odds.posterior:.9f}")
    print(f"  d(posterior)/d(prior):    {sensitivity.d_posterior_d_prior:.9f}")
    print(f"  model posterior:          {comparison.model_posterior:.9f}")
    print(f"  explicit posterior:       {comparison.explicit_posterior:.9f}")
    print(f"  explicit - model:         {comparison.difference:.9f}")
    print(f"  threshold 0.6 met:        {threshold.threshold_met}")
    print(
        "  stress minimum posterior: "
        f"{stress.minimum_posterior_scenario.posterior:.9f}"
    )
    print(
        "  stress maximum posterior: "
        f"{stress.maximum_posterior_scenario.posterior:.9f}"
    )
    print(f"  original unchanged:       {result.posterior == calculation.posterior}")
    print()


def time_operation(
    name: str,
    operation: Callable[[], float],
    iterations: int,
    scope: str,
) -> Benchmark:
    warmup_iterations = min(iterations, 1_000)
    checksum = 0.0
    for _ in range(warmup_iterations):
        checksum += operation()

    samples: list[float] = []
    for _ in range(7):
        started = time.perf_counter_ns()
        for _ in range(iterations):
            checksum += operation()
        elapsed = time.perf_counter_ns() - started
        samples.append(elapsed / iterations)

    posterior = checksum / (warmup_iterations + (7 * iterations))
    return Benchmark(name, statistics.median(samples), iterations, posterior, scope)


def unchecked_python_formula() -> float:
    prior = 0.4
    p_e_given_h = 0.8
    p_e_given_not_h = 0.2
    numerator = prior * p_e_given_h
    return numerator / (numerator + ((1.0 - prior) * p_e_given_not_h))


def sklearn_operation() -> tuple[Callable[[], float], str] | None:
    try:
        import numpy
        import sklearn
        from sklearn.naive_bayes import BernoulliNB
    except ImportError:
        return None

    # Integer counts encode prior=0.4, P(E|H)=0.8, and P(E|not H)=0.2.
    observations = numpy.array(
        ([[1.0]] * 12)
        + ([[0.0]] * 48)
        + ([[1.0]] * 32)
        + ([[0.0]] * 8)
    )
    classes = numpy.array(([0] * 60) + ([1] * 40))
    try:
        model = BernoulliNB(alpha=0.0, force_alpha=True)
    except TypeError:
        model = BernoulliNB(alpha=0.0)
    model.fit(observations, classes)
    sample = numpy.array([[1.0]])

    def infer() -> float:
        return float(model.predict_proba(sample)[0, 1])

    expected = 8.0 / 11.0
    if not math.isclose(infer(), expected, rel_tol=1e-9, abs_tol=1e-12):
        raise RuntimeError("scikit-learn comparison model has the wrong posterior")
    return infer, sklearn.__version__


def run_cpp_demo(executable: Path, iterations: int) -> list[Benchmark]:
    completed = subprocess.run(
        [str(executable), str(iterations)],
        check=True,
        capture_output=True,
        text=True,
    )
    print(completed.stdout.rstrip())
    results: list[Benchmark] = []
    scopes = {
        "cpp_unchecked_formula": "unchecked arithmetic latency floor",
        "bbus_cpp_core": "validated C++ mathematical core",
        "bbus_cpp_audited_update": "complete C++ validate-resolve-update-result path",
    }
    for line in completed.stdout.splitlines():
        if not line.startswith("BENCHMARK,"):
            continue
        _, name, nanoseconds, count, posterior = line.split(",")
        results.append(
            Benchmark(
                name,
                float(nanoseconds),
                int(count),
                float(posterior),
                scopes[name],
            )
        )
    if len(results) != 3:
        raise RuntimeError("C++ demo did not emit all benchmark records")
    return results


def format_duration(nanoseconds: float) -> str:
    if nanoseconds < 1_000.0:
        return f"{nanoseconds:.1f} ns"
    if nanoseconds < 1_000_000.0:
        return f"{nanoseconds / 1_000.0:.2f} us"
    return f"{nanoseconds / 1_000_000.0:.2f} ms"


def write_csv(results: list[Benchmark], destination: Path) -> None:
    with destination.open("w", newline="", encoding="utf-8") as output:
        writer = csv.writer(output, lineterminator="\n")
        writer.writerow(
            ["implementation", "median_ns_per_update", "iterations", "posterior", "scope"]
        )
        writer.writerows(results)


def write_markdown(
    results: list[Benchmark], destination: Path, sklearn_version: str | None
) -> None:
    lines = [
        "# Demo benchmark results",
        "",
        "Median of seven trials. Lower latency is better.",
        "",
        "| Implementation | Median per update | Iterations/trial | Posterior | Scope |",
        "|---|---:|---:|---:|---|",
    ]
    for result in sorted(results, key=lambda item: item.median_ns):
        lines.append(
            f"| `{result.name}` | {format_duration(result.median_ns)} | "
            f"{result.iterations:,} | {result.posterior:.9f} | {result.scope} |"
        )
    lines.extend(
        [
            "",
            f"- Platform: `{platform.platform()}`",
            f"- Python: `{platform.python_version()}`",
            f"- BBUS: `{belief_update.__version__}`",
            f"- scikit-learn: `{sklearn_version or 'not installed; comparison skipped'}`",
            "",
            "These are illustrative local measurements, not universal performance claims. "
            "The scikit-learn row measures scalar `BernoulliNB.predict_proba`; it does not "
            "provide BBUS provenance, validation, replay, or stress-analysis semantics. "
            "Unchecked formula rows omit safety and audit behavior.",
            "",
        ]
    )
    destination.write_text("\n".join(lines), encoding="utf-8")


def write_svg(results: list[Benchmark], destination: Path) -> None:
    ordered = sorted(results, key=lambda item: item.median_ns)
    width = 1_080
    left = 295
    right = 175
    top = 105
    row_height = 58
    bottom = 90
    chart_width = width - left - right
    height = top + (row_height * len(ordered)) + bottom
    minimum_power = math.floor(math.log10(min(item.median_ns for item in ordered)))
    maximum_power = math.ceil(math.log10(max(item.median_ns for item in ordered)))
    if minimum_power == maximum_power:
        maximum_power += 1

    def x_position(value: float) -> float:
        fraction = (math.log10(value) - minimum_power) / (
            maximum_power - minimum_power
        )
        return left + (fraction * chart_width)

    colors = {
        "bbus_cpp_core": "#2f6fed",
        "bbus_cpp_audited_update": "#1854b4",
        "bbus_python_core_binding": "#23a98c",
        "bbus_python_audited_update": "#147a65",
        "sklearn_bernoulli_nb_scalar": "#e28a24",
    }
    svg = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#fbfaf7"/>',
        '<text x="40" y="42" font-family="system-ui, sans-serif" font-size="24" font-weight="700" fill="#172033">BBUS demo latency comparison</text>',
        '<text x="40" y="70" font-family="system-ui, sans-serif" font-size="14" fill="#536078">Median ns per single update - lower is better - logarithmic scale</text>',
    ]
    for power in range(minimum_power, maximum_power + 1):
        value = 10.0**power
        x = x_position(value)
        svg.append(
            f'<line x1="{x:.2f}" y1="{top - 15}" x2="{x:.2f}" y2="{height - bottom + 10}" stroke="#d9dde5" stroke-width="1"/>'
        )
        svg.append(
            f'<text x="{x:.2f}" y="{height - 42}" text-anchor="middle" font-family="ui-monospace, monospace" font-size="12" fill="#667085">{html.escape(format_duration(value))}</text>'
        )

    for index, result in enumerate(ordered):
        y = top + (index * row_height)
        end_x = x_position(result.median_ns)
        color = colors.get(result.name, "#7a8394")
        svg.append(
            f'<text x="{left - 14}" y="{y + 23}" text-anchor="end" font-family="ui-monospace, monospace" font-size="13" fill="#263247">{html.escape(result.name)}</text>'
        )
        svg.append(
            f'<rect x="{left}" y="{y + 5}" width="{max(3.0, end_x - left):.2f}" height="26" rx="4" fill="{color}"/>'
        )
        svg.append(
            f'<text x="{min(width - right + 12, end_x + 10):.2f}" y="{y + 23}" font-family="ui-monospace, monospace" font-size="13" font-weight="700" fill="#172033">{html.escape(format_duration(result.median_ns))}</text>'
        )
    svg.extend(
        [
            f'<text x="40" y="{height - 12}" font-family="system-ui, sans-serif" font-size="12" fill="#667085">Local illustrative benchmark; scopes differ. See benchmark_results.md.</text>',
            "</svg>",
        ]
    )
    destination.write_text("\n".join(svg), encoding="utf-8")


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--iterations", type=int, default=10_000)
    parser.add_argument("--cpp-iterations", type=int, default=200_000)
    parser.add_argument("--cpp-executable", type=Path)
    parser.add_argument(
        "--output-dir", type=Path, default=Path(__file__).parent / "results"
    )
    parser.add_argument("--skip-sklearn", action="store_true")
    arguments = parser.parse_args()
    if arguments.iterations <= 0 or arguments.cpp_iterations <= 0:
        parser.error("iteration counts must be positive")
    return arguments


def main() -> int:
    arguments = parse_arguments()
    showcase()

    prior = belief_update.BeliefState(probability=0.4)
    likelihoods = belief_update.LikelihoodValues(
        evidence_given_hypothesis=0.8,
        evidence_given_not_hypothesis=0.2,
    )
    results = [
        time_operation(
            "python_unchecked_formula",
            unchecked_python_formula,
            arguments.iterations,
            "unchecked Python arithmetic latency floor",
        ),
        time_operation(
            "bbus_python_core_binding",
            lambda: belief_update.exact_binary_update(
                prior=prior, likelihoods=likelihoods
            ).posterior,
            arguments.iterations,
            "Python-to-C++ mathematical core binding",
        ),
        time_operation(
            "bbus_python_audited_update",
            lambda: belief_update.update(
                prior=0.4, p_e_given_h=0.8, p_e_given_not_h=0.2
            ).posterior,
            arguments.iterations,
            "Python-to-C++ complete validate-resolve-update-result path",
        ),
    ]

    sklearn_version: str | None = None
    if not arguments.skip_sklearn:
        comparison = sklearn_operation()
        if comparison is None:
            print(
                "scikit-learn is not installed; its optional comparison was skipped."
            )
        else:
            operation, sklearn_version = comparison
            results.append(
                time_operation(
                    "sklearn_bernoulli_nb_scalar",
                    operation,
                    arguments.iterations,
                    "scalar BernoulliNB.predict_proba on equivalent evidence",
                )
            )

    if arguments.cpp_executable is not None:
        results.extend(
            run_cpp_demo(arguments.cpp_executable, arguments.cpp_iterations)
        )

    print("\nMedian of 7 trials (lower is better)")
    for result in sorted(results, key=lambda item: item.median_ns):
        print(f"  {result.name:<31} {format_duration(result.median_ns):>10}")

    arguments.output_dir.mkdir(parents=True, exist_ok=True)
    csv_path = arguments.output_dir / "benchmark_results.csv"
    markdown_path = arguments.output_dir / "benchmark_results.md"
    svg_path = arguments.output_dir / "benchmark_results.svg"
    write_csv(results, csv_path)
    write_markdown(results, markdown_path, sklearn_version)
    write_svg(results, svg_path)
    print(f"\nWrote {csv_path}, {markdown_path}, and {svg_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
