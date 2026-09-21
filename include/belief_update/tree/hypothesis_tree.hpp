#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace belief_update {

class HypothesisId final {
 public:
  explicit constexpr HypothesisId(std::uint32_t value) noexcept
      : value_(value) {}

  [[nodiscard]] constexpr std::uint32_t value() const noexcept {
    return value_;
  }

  friend constexpr bool operator==(HypothesisId, HypothesisId) = default;

 private:
  std::uint32_t value_;
};

class HypothesisTree final {
 public:
  // Node IDs are their positions. Node zero is the root; every other parent
  // must precede its child. This makes connectivity and acyclicity explicit.
  explicit HypothesisTree(
      std::vector<std::optional<HypothesisId>> parents);

  HypothesisTree(const HypothesisTree&) = default;
  HypothesisTree(HypothesisTree&&) noexcept = default;
  HypothesisTree& operator=(const HypothesisTree&) = delete;
  HypothesisTree& operator=(HypothesisTree&&) = delete;

  [[nodiscard]] std::size_t node_count() const noexcept {
    return parents_.size();
  }
  [[nodiscard]] std::span<const std::optional<HypothesisId>> parents()
      const noexcept {
    return parents_;
  }
  [[nodiscard]] const std::optional<HypothesisId>& parent(
      HypothesisId node) const;
  [[nodiscard]] std::span<const HypothesisId> leaf_ids() const noexcept {
    return leaf_ids_;
  }
  [[nodiscard]] bool is_leaf(HypothesisId node) const;

 private:
  [[nodiscard]] std::size_t checked_index(HypothesisId node) const;

  std::vector<std::optional<HypothesisId>> parents_;
  std::vector<HypothesisId> leaf_ids_;
};

}  // namespace belief_update
