#include "belief_update/tree/replay.hpp"

#include <stdexcept>

#include "belief_update/tree/update.hpp"
#include "belief_update/version.hpp"

namespace belief_update {

TreeUpdateResult replay(const TreeUpdateResult& recorded) {
  if (recorded.library_version() != version) {
    throw std::invalid_argument(
        "recorded tree result uses a different library version");
  }
  return update_tree(recorded.prior(), recorded.likelihoods(),
                     recorded.provenance());
}

}  // namespace belief_update
