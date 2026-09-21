#include <cstdlib>

#include "belief_update/version.hpp"

int main() {
  return belief_update::version == "0.1.0" ? EXIT_SUCCESS : EXIT_FAILURE;
}
