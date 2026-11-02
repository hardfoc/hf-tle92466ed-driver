/**
 * @file main.cpp
 * @brief Builds the driver from an installed package only: headers, version header and .ipp.
 *
 * Reuses the documented port (tests/docs/my_spi.hpp); its platform hooks are declared only, so the
 * consumer is an object library: the check is that every driver member compiles from the install.
 */
#include "../docs/my_spi.hpp"
#include "tle92466ed_version.h"

template class tle92466ed::Driver<MySpi>;
