#include <cstdlib>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#include "Constants.h"
#include "Game.h"


// Should make it so explosions dont cause "hitches" on some GPUs/drivers...
std::vector<sf::Texture> prerender_explosion_textures(const sf::Vector2f& explosion_size)
{
	std::vector<sf::Texture> explosion_anim_textures;
	printf("Pre-rendering explosion animation textures..\n");

	auto explosion_density = 1.0f;
	while (explosion_density > 0.0f) {
		printf("Pre-rendering explosion anim frame %d ..\n", static_cast<int>(explosion_anim_textures.size()));

		sf::RenderTexture explosion_texture;
		explosion_texture.create(static_cast<unsigned int>(explosion_size.x), static_cast<unsigned int>(explosion_size.y));
		explosion_texture.clear(sf::Color(0, 0, 0, 0));

		sf::CircleShape explosion_outer(0.5f);
		explosion_outer.setScale(explosion_size);
		explosion_outer.setFillColor(sf::Color(255, static_cast<sf::Uint8>(explosion_density * 255), 0, static_cast<sf::Uint8>(explosion_density * 255)));
		explosion_texture.draw(explosion_outer);

		sf::CircleShape explosion_inner(0.5f);
		explosion_inner.setScale((1.0f - explosion_density) * explosion_size);
		explosion_inner.setPosition(0.5f * explosion_density * explosion_size);
		explosion_inner.setFillColor(sf::Color(0, 0, 0, 0));
		explosion_texture.draw(explosion_inner, sf::BlendNone); // mask for cool effect

		explosion_texture.display();
		explosion_anim_textures.emplace_back(explosion_texture.getTexture());
		explosion_density -= 2.0f * Constants::FRAME_TIME.asSeconds();
	}

	printf("Finished pre-rendering explosion textures! Rendered %d frames.\n", static_cast<int>(explosion_anim_textures.size()));
	return explosion_anim_textures;
}


int main(int argc, char* argv[])
{
	sf::Font font;
	if (!font.loadFromFile("GameFont.ttf")) {
		fprintf(stderr, "ERROR: Failed to load game font! Make sure that there is a font called \"GameFont.ttf\" in the working directory please!\n");
		fprintf(stderr, "(It doesn't even have to be a TrueType font - just make sure it has the \".ttf\" extension anyway)\n");
		return EXIT_FAILURE;
	}

	sf::RenderWindow window(sf::VideoMode(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT), "Sean's MA3513 Project Demo - City Defender");
	window.setFramerateLimit(Constants::FRAME_RATE);

	const auto explosion_anim_textures = prerender_explosion_textures(sf::Vector2f(100.0f, 100.0f));
	Game game(font, &explosion_anim_textures);

	while (window.isOpen()) {
		// handle window message queue
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				printf("Window has been closed - stopping.. (might take a while to deallocate everything) \n");
				break;
			}
		}

		game.tick(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
		game.render(window);

		window.display();
	}

	return EXIT_SUCCESS;
}