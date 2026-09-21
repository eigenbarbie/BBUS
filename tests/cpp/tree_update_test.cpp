#include <cstdint>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "belief_update/tree.hpp"
#include "belief_update/types/provenance.hpp"
#include "belief_update/version.hpp"
#include "test_support.hpp"

namespace {

[[nodiscard]] belief_update::HypothesisTree example_tree() {
  using belief_update::HypothesisId;
  return belief_update::HypothesisTree{
      std::vector<std::optional<HypothesisId>>{
          std::nullopt, HypothesisId{0}, HypothesisId{0}, HypothesisId{2},
          HypothesisId{2}}};
}

[[nodiscard]] belief_update::Provenance example_provenance() {
  using namespace belief_update;
  return Provenance{SourceType::model, SourceId{"tree-model"},
                    SourceVersion{"1"}, ProvenanceTimestamp{1'000}, 0.9,
                    OverrideStatus::not_requested};
}

}  // namespace

int main() {
  using namespace belief_update;
  constexpr double tolerance = 1e-12;

  static_assert(!std::is_copy_assignable_v<TreeUpdateResult>);
  static_assert(!std::is_move_assignable_v<TreeUpdateResult>);

  const HypothesisTree tree = example_tree();
  test_support::expect(tree.node_count() == 5, "tree retains every node");
  test_support::expect(tree.leaf_ids().size() == 3,
                       "tree identifies all leaves");
  test_support::expect(tree.leaf_ids()[0] == HypothesisId{1},
                       "leaf order is deterministic");
  test_support::expect(tree.is_leaf(HypothesisId{4}),
                       "leaf classification is available");
  test_support::expect(!tree.is_leaf(HypothesisId{2}),
                       "internal classification is available");

  const TreeBeliefState prior{tree, {0.5, 0.2, 0.3}, tolerance};
  test_support::expect_near(prior.probability(HypothesisId{0}), 1.0,
                            tolerance, "root prior is aggregated");
  test_support::expect_near(prior.probability(HypothesisId{2}), 0.5,
                            tolerance, "internal prior is aggregated");

  const TreeUpdateResult result = update_tree(
      prior, TreeLikelihoodValues{{0.1, 0.8, 0.4}}, example_provenance());
  const TreeBeliefState& posterior = result.posterior();
  test_support::expect_near(result.likelihood_scale(), 0.8, tolerance,
                            "likelihood scaling is recorded");
  test_support::expect_near(result.scaled_evidence_mass(), 0.4125, tolerance,
                            "scaled evidence mass is recorded");
  test_support::expect_near(posterior.probability(HypothesisId{1}), 5.0 / 33.0,
                            tolerance, "first leaf posterior is exact");
  test_support::expect_near(posterior.probability(HypothesisId{3}),
                            16.0 / 33.0, tolerance,
                            "nested leaf posterior is exact");
  test_support::expect_near(posterior.probability(HypothesisId{4}),
                            12.0 / 33.0, tolerance,
                            "last leaf posterior is exact");
  test_support::expect_near(posterior.probability(HypothesisId{2}),
                            28.0 / 33.0, tolerance,
                            "internal posterior sums descendants");
  test_support::expect_near(posterior.probability(HypothesisId{0}), 1.0,
                            tolerance, "root posterior sums all leaves");
  test_support::expect(result.provenance().source_id().value() == "tree-model",
                       "tree result preserves provenance");
  test_support::expect(result.library_version() == version,
                       "tree result records the library version");

  const TreeUpdateResult reproduced = replay(result);
  for (std::size_t index = 0; index < tree.node_count(); ++index) {
    const HypothesisId node{static_cast<std::uint32_t>(index)};
    test_support::expect(
        reproduced.posterior().probability(node) == posterior.probability(node),
        "tree replay exactly reproduces node posteriors");
  }

  const TreeUpdateResult tiny = update_tree(
      prior, TreeLikelihoodValues{{1e-300, 2e-300, 4e-300}},
      example_provenance());
  test_support::expect_near(tiny.posterior().probability(HypothesisId{0}), 1.0,
                            tolerance,
                            "likelihood scaling handles tiny probabilities");

  test_support::expect_throws<std::invalid_argument>(
      [] { static_cast<void>(HypothesisTree{{}}); },
      "a tree requires a root");
  test_support::expect_throws<std::invalid_argument>(
      [] {
        static_cast<void>(HypothesisTree{
            {std::nullopt, std::nullopt, HypothesisId{0}}});
      },
      "a tree rejects a second root");
  test_support::expect_throws<std::invalid_argument>(
      [] {
        static_cast<void>(
            HypothesisTree{{std::nullopt, HypothesisId{1}, HypothesisId{0}}});
      },
      "a tree rejects forward and cyclic parents");
  test_support::expect_throws<std::invalid_argument>(
      [] {
        static_cast<void>(
            HypothesisTree{{std::nullopt, HypothesisId{0}}});
      },
      "a tree requires at least two possibilities");
  test_support::expect_throws<std::invalid_argument>(
      [&tree] {
        static_cast<void>(TreeBeliefState{tree, {0.5, 0.5}, tolerance});
      },
      "prior count must match leaves");
  test_support::expect_throws<std::invalid_argument>(
      [&tree] {
        static_cast<void>(TreeBeliefState{tree, {0.4, 0.2, 0.3}, tolerance});
      },
      "priors are never silently normalized");
  test_support::expect_throws<std::invalid_argument>(
      [&prior] {
        static_cast<void>(update_tree(prior, TreeLikelihoodValues{{0.5, 0.5}},
                                      example_provenance()));
      },
      "likelihood count must match leaves");
  test_support::expect_throws<std::domain_error>(
      [&prior] {
        static_cast<void>(
            update_tree(prior, TreeLikelihoodValues{{0.0, 0.0, 0.0}},
                        example_provenance()));
      },
      "zero likelihood under all leaves is rejected");
  test_support::expect_throws<std::domain_error>(
      [&tree] {
        const TreeBeliefState certain{tree, {1.0, 0.0, 0.0}, tolerance};
        static_cast<void>(
            update_tree(certain, TreeLikelihoodValues{{0.0, 0.5, 0.5}},
                        example_provenance()));
      },
      "zero evidence probability under the prior is rejected");
  test_support::expect_throws<std::invalid_argument>(
      [&prior] {
        const Provenance malformed{
            static_cast<SourceType>(255), SourceId{"tree-model"},
            SourceVersion{"1"}, ProvenanceTimestamp{1'000}, 0.9,
            OverrideStatus::not_requested};
        static_cast<void>(update_tree(
            prior, TreeLikelihoodValues{{0.1, 0.8, 0.4}}, malformed));
      },
      "malformed tree provenance is rejected");

  return test_support::finish();
}
