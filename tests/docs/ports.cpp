/**
 * @file ports.cpp
 * @brief Instantiates the driver over the documented port, so every member compiles in full.
 */
#include "my_spi.hpp"

template class tle92466ed::Driver<MySpi>;
