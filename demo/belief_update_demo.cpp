#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "belief_update/analysis/local_sensitivity.hpp"
#include "belief_update/analysis/posterior_comparison.hpp"
#include "belief_update/analysis/threshold_analysis.hpp"
#include "belief_update/core/bayes.hpp"
#include "belief_update/replay/replay.hpp"
#include "belief_update/stress/stress.hpp"
#include "belief_update/tree.hpp"
#include "belief_update/types.hpp"
#include "belief_update/update/update.hpp"

namespace {

using Clock = std::chrono::steady_clock;

struct BenchmarkResult {
  std::string_view name;
  double nanoseconds_per_update;
  double posterior;
};

[[nodiscard]] belief_update::LikelihoodInput make_input(
    double p_e_given_h, double p_e_given_not_h,
    belief_update::SourceType source_type, std::string source_id,
    belief_update::OverrideStatus override_status) {
  using namespace belief_update;
  return LikelihoodInput{
      LikelihoodValues{p_e_given_h, p_e_given_not_h}, std::nullopt,
      Provenance{source_type, SourceId{std::move(source_id)},
                 SourceVersion{"demo-v1"}, ProvenanceTimestamp{1'000}, 0.95,
                 override_status}};
}

[[nodiscard]] belief_update::Provenance make_tree_provenance() {
  using namespace belief_update;
  return Provenance{SourceType::model, SourceId{"transport-model"},
                    SourceVersion{"demo-v1"}, ProvenanceTimestamp{1'000}, 0.95,
                    OverrideStatus::not_requested};
}

[[nodiscard]] double unchecked_formula(double prior, double p_e_given_h,
                                       double p_e_given_not_h) noexcept {
  const double numerator = prior * p_e_given_h;
  return numerator /
         (numerator + ((1.0 - prior) * p_e_given_not_h));
}

template <typename Operation>
[[nodiscard]] BenchmarkResult benchmark(std::string_view name,
                                        std::size_t iterations,
                                        Operation operation) {
  constexpr std::size_t trial_count = 7;
  const std::size_t warmup_iterations =
      std::min<std::size_t>(iterations, 10'000);
  double checksum = 0.0;
  for (std::size_t index = 0; index < warmup_iterations; ++index) {
    checksum += operation();
  }

  std::vector<double> samples;
  samples.reserve(trial_count);
  for (std::size_t trial = 0; trial < trial_count; ++trial) {
    const auto started = Clock::now();
    for (std::size_t index = 0; index < iterations; ++index) {
      checksum += operation();
    }
    const auto elapsed = Clock::now() - started;
    const double elapsed_nanoseconds =
        std::chrono::duration<double, std::nano>{elapsed}.count();
    samples.push_back(elapsed_nanoseconds / static_cast<double>(iterations));
  }
  std::sort(samples.begin(), samples.end());

  // Printing a checksum after timing keeps the optimizer from discarding work.
  const double posterior = checksum /
                           static_cast<double>(warmup_iterations +
                                               (trial_count * iterations));
  return BenchmarkResult{name, samples[trial_count / 2], posterior};
}

[[nodiscard]] std::size_t parse_iterations(int argument_count,
                                           char* arguments[]) {
  constexpr std::size_t default_iterations = 200'000;
  if (argument_count == 1) {
    return default_iterations;
  }
  if (argument_count != 2) {
    throw std::invalid_argument(
        "usage: belief_update_demo [positive-iteration-count]");
  }

  std::size_t parsed_characters = 0;
  const unsigned long long parsed =
      std::stoull(arguments[1], &parsed_characters, 10);
  if (parsed_characters != std::string{arguments[1]}.size() || parsed == 0) {
    throw std::invalid_argument("iteration count must be a positive integer");
  }
  return static_cast<std::size_t>(parsed);
}

void print_benchmark(const BenchmarkResult& result,
                     std::size_t iterations) {
  std::cout << std::left << std::setw(31) << result.name << std::right
            << std::setw(12) << std::fixed << std::setprecision(2)
            << result.nanoseconds_per_update << " ns/update\n";
  std::cout << "BENCHMARK," << result.name << ',' << std::setprecision(17)
            << result.nanoseconds_per_update << ',' << iterations << ','
            << result.posterior << '\n';
}

}  // namespace

int main(int argument_count, char* arguments[]) {
  using namespace belief_update;

  try {
    const std::size_t iterations =
        parse_iterations(argument_count, arguments);
    const LikelihoodInput explicit_input =
        make_input(0.8, 0.3, SourceType::human, "analyst-7",
                   OverrideStatus::requested);
    const LikelihoodInput model_input =
        make_input(0.8, 0.2, SourceType::model, "model-a",
                   OverrideStatus::not_requested);
    const UpdateConfig configuration{std::nullopt};
    const UpdateResult result = update(
        UpdateInputs{BeliefState{0.4}, explicit_input, model_input,
                     ProvenanceTimestamp{1'000}},
        configuration, UpdateAlgorithm::exact_binary);

    const BayesCalculation& calculation = result.intermediate_calculations();
    const UpdateResult reproduced = replay(result);
    const LocalSensitivityResult sensitivity = local_sensitivity(result);
    const PosteriorComparisonResult comparison =
        compare_model_and_explicit(result);
    const ThresholdAnalysisResult threshold = analyze_threshold(result, 0.6);
    const StressResult stress = stress_update(
        result, RelativeStressRanges{10.0, 10.0, 10.0}, 0.7);
    const BayesCalculation log_odds =
        log_odds_update(BeliefState{0.4}, std::log(0.8 / 0.2));
    const HypothesisTree tree{
        {std::nullopt, HypothesisId{0}, HypothesisId{0}, HypothesisId{2},
         HypothesisId{2}}};
    const TreeBeliefState tree_prior{tree, {0.5, 0.2, 0.3}, 1e-12};
    const TreeLikelihoodValues tree_likelihoods{{0.1, 0.8, 0.4}};
    const Provenance tree_provenance = make_tree_provenance();
    const TreeUpdateResult tree_result =
        update_tree(tree_prior, tree_likelihoods, tree_provenance);
    const TreeUpdateResult replayed_tree = replay(tree_result);

    std::cout << std::setprecision(9);
    std::cout << "BBUS C++ capability demo\n"
              << "  prior:                    " << calculation.prior() << '\n'
              << "  likelihood ratio:         "
              << calculation.likelihood_ratio() << '\n'
              << "  posterior:                " << result.posterior() << '\n'
              << "  belief delta:             " << calculation.belief_delta()
              << '\n'
              << "  provenance source id:     "
              << result.provenance().source_id().value() << '\n'
              << "  replay identical:         " << std::boolalpha
              << (reproduced.posterior() == result.posterior()) << '\n'
              << "  log-odds posterior:       " << log_odds.posterior() << '\n'
              << "  d(posterior)/d(prior):    "
              << sensitivity.d_posterior_d_prior() << '\n'
              << "  model posterior:          "
              << comparison.model_posterior() << '\n'
              << "  explicit posterior:       "
              << comparison.explicit_posterior() << '\n'
              << "  explicit - model:         " << comparison.difference()
              << '\n'
              << "  threshold 0.6 met:        " << threshold.threshold_met()
              << '\n'
              << "  stress minimum posterior: "
              << stress.minimum_posterior_scenario().posterior() << '\n'
              << "  stress maximum posterior: "
              << stress.maximum_posterior_scenario().posterior() << '\n'
              << "  original unchanged:       "
              << (result.posterior() == calculation.posterior()) << '\n'
              << "  tree car posterior:       "
              << tree_result.posterior().probability(HypothesisId{1}) << '\n'
              << "  tree public transit:      "
              << tree_result.posterior().probability(HypothesisId{2}) << '\n'
              << "  tree bus posterior:       "
              << tree_result.posterior().probability(HypothesisId{3}) << '\n'
              << "  tree train posterior:     "
              << tree_result.posterior().probability(HypothesisId{4}) << '\n'
              << "  tree replay identical:    "
              << std::equal(
                     replayed_tree.posterior().node_probabilities().begin(),
                     replayed_tree.posterior().node_probabilities().end(),
                     tree_result.posterior().node_probabilities().begin(),
                     tree_result.posterior().node_probabilities().end())
              << "\n\n";

    volatile double benchmark_prior = 0.4;
    const BeliefState core_prior{0.4};
    const LikelihoodValues core_likelihoods{0.8, 0.2};
    const BenchmarkResult formula_result = benchmark(
        "cpp_unchecked_formula", iterations, [&benchmark_prior] {
          return unchecked_formula(benchmark_prior, 0.8, 0.2);
        });
    const BenchmarkResult core_result = benchmark(
        "bbus_cpp_core", iterations, [&core_prior, &core_likelihoods] {
          return exact_binary_update(core_prior, core_likelihoods).posterior();
        });
    const BenchmarkResult pipeline_result = benchmark(
        "bbus_cpp_audited_update", iterations, [&benchmark_prior] {
          return update(benchmark_prior, 0.8, 0.2).posterior();
        });
    const BenchmarkResult tree_result_benchmark = benchmark(
        "bbus_cpp_tree_update", iterations,
        [&tree_prior, &tree_likelihoods, &tree_provenance] {
          return update_tree(tree_prior, tree_likelihoods, tree_provenance)
              .posterior()
              .probability(HypothesisId{2});
        });

    std::cout << "Median of 7 trials (" << iterations
              << " updates per trial; lower is better)\n";
    print_benchmark(formula_result, iterations);
    print_benchmark(core_result, iterations);
    print_benchmark(pipeline_result, iterations);
    print_benchmark(tree_result_benchmark, iterations);
    std::cout << "\nThe unchecked formula is a latency floor, not a safe or "
                 "auditable replacement.\n";
  } catch (const std::exception& exception) {
    std::cerr << "demo error: " << exception.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
