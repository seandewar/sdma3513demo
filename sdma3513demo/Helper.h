#pragma once

#include <random>

class Helper
{
	static std::mt19937 rng_;

public:
	static inline int get_random_int(std::mt19937& rng, int min, int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng);
	}
	static inline int get_random_int(int min, int max) { return get_random_int(rng_, min, max); }

	static inline float get_random_float(std::mt19937& rng, float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng);
	}
	static inline float get_random_float(float min, float max) { return get_random_float(rng_, min, max); }

	static inline bool get_random_bool(std::mt19937& rng, double true_chance)
	{
		std::bernoulli_distribution dist(true_chance);
		return dist(rng);
	}
	static inline bool get_random_bool(double true_chance) { return get_random_bool(rng_, true_chance); }
};