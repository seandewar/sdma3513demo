#include "Helper.h"

#include <chrono>


std::mt19937 Helper::rng_(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));