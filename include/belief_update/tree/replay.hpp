#pragma once

#include "belief_update/tree/tree_update_result.hpp"

namespace belief_update {

[[nodiscard]] TreeUpdateResult replay(const TreeUpdateResult& recorded);

}  // namespace belief_update
