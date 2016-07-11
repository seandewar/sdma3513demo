#pragma once

#include "SFML/System/Time.hpp"

namespace Constants
{
	const unsigned int VIDEO_WIDTH = 1024;
	const unsigned int VIDEO_HEIGHT = 576;

	const unsigned int FRAME_RATE = 30;
	const auto FRAME_TIME = sf::seconds(1.0f / FRAME_RATE);
}