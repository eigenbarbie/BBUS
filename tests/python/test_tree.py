import unittest

import belief_update


def make_tree() -> belief_update.HypothesisTree:
    return belief_update.HypothesisTree(
        parents=[
            None,
            belief_update.HypothesisId(value=0),
            belief_update.HypothesisId(value=0),
            belief_update.HypothesisId(value=2),
            belief_update.HypothesisId(value=2),
        ]
    )


def make_provenance() -> belief_update.Provenance:
    return belief_update.Provenance(
        source_type=belief_update.SourceType.model,
        source_id=belief_update.SourceId(value="tree-model"),
        version=belief_update.SourceVersion(value="1"),
        timestamp=belief_update.ProvenanceTimestamp(unix_nanoseconds=1_000),
        confidence=0.9,
        override_status=belief_update.OverrideStatus.not_requested,
    )


class TreeUpdateTest(unittest.TestCase):
    def test_nested_tree_updates_and_replays(self) -> None:
        tree = make_tree()
        prior = belief_update.TreeBeliefState(
            tree=tree,
            leaf_probabilities=[0.5, 0.2, 0.3],
            probability_sum_tolerance=1e-12,
        )
        result = belief_update.update_tree(
            prior=prior,
            likelihoods=belief_update.TreeLikelihoodValues(
                evidence_given_leaves=[0.1, 0.8, 0.4]
            ),
            provenance=make_provenance(),
        )

        self.assertEqual([node.value for node in tree.leaf_ids], [1, 3, 4])
        self.assertAlmostEqual(
            result.posterior.probability(belief_update.HypothesisId(1)),
            5.0 / 33.0,
        )
        self.assertAlmostEqual(
            result.posterior.probability(belief_update.HypothesisId(2)),
            28.0 / 33.0,
        )
        self.assertAlmostEqual(
            result.posterior.probability(belief_update.HypothesisId(0)), 1.0
        )
        self.assertEqual(result.provenance.source_id.value, "tree-model")
        self.assertEqual(result.library_version, belief_update.__version__)

        replayed = belief_update.replay(recorded=result)
        self.assertEqual(
            replayed.posterior.node_probabilities,
            result.posterior.node_probabilities,
        )
        with self.assertRaises(AttributeError):
            result.likelihood_scale = 1.0

    def test_invalid_tree_inputs_fail_without_correction(self) -> None:
        tree = make_tree()
        with self.assertRaises(ValueError):
            belief_update.TreeBeliefState(
                tree=tree,
                leaf_probabilities=[0.4, 0.2, 0.3],
                probability_sum_tolerance=1e-12,
            )
        with self.assertRaises(ValueError):
            belief_update.TreeBeliefState(
                tree=tree,
                leaf_probabilities=[0.5, 0.2, 0.3],
                probability_sum_tolerance=-1.0,
            )

        prior = belief_update.TreeBeliefState(
            tree=tree,
            leaf_probabilities=[0.5, 0.2, 0.3],
            probability_sum_tolerance=1e-12,
        )
        with self.assertRaises(ValueError):
            belief_update.update_tree(
                prior=prior,
                likelihoods=belief_update.TreeLikelihoodValues(
                    evidence_given_leaves=[0.0, 0.0, 0.0]
                ),
                provenance=make_provenance(),
            )


if __name__ == "__main__":
    unittest.main()
