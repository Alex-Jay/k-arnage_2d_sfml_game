//#include "Application.hpp"
//
//#include <iostream>
//
////#include "MapTiler.hpp"
//
//int main()
//{
//	try
//	{
//		Application app;
//		app.run();
//	}
//	catch (std::exception e)
//	{
//		std::cout << "\n EXCEPTION: " << e.what() << std::endl;
//	}
//}
#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
	// _____________________
	// ::: Create window :::

	// Create a window with the same pixel depth as the desktop
	sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();

	sf::RenderWindow window(sf::VideoMode(1024, 768), "K-ARNAGE", sf::Style::Close);

	// Enable vertical sync. (vsync)
	window.setVerticalSyncEnabled(true);



	// ____________________
	// ::: Load texture :::

	// Create texture from PNG file
	sf::Texture texture;
	if (!texture.loadFromFile("Water.jpg"))
	{
		std::cerr << "Error while loading texture" << std::endl;
		return -1;
	}
	// Enable the smooth filter. The texture appears smoother so that pixels are less noticeable.
	texture.setSmooth(true);



	// _______________________________________
	// ::: Create sprite and apply texture :::

	// Create the sprite and apply the texture
	sf::Sprite sprite;
	sprite.setTexture(texture);
	sf::FloatRect spriteSize = sprite.getGlobalBounds();
	// Set origin in the middle of the sprite
	sprite.setOrigin(spriteSize.width / 2., spriteSize.height / 2.);



	// _______________
	// ::: Shaders :::

	// Check if shaders are available
	if (!sf::Shader::isAvailable())
	{
		std::cerr << "Shader are not available" << std::endl;
		return -1;
	}

	// Load shaders
	sf::Shader shader;
	if (!shader.loadFromFile("example_001.frag", sf::Shader::Fragment))
	{
		std::cerr << "Error while shaders" << std::endl;
		return -1;
	}



	// ___________________________
	// ::: Load distortion map :::

	sf::Texture distortionMap;

	if (!distortionMap.loadFromFile("distortion_map.png"))
	{
		sf::err() << "Error while loading distortion map" << std::endl;
		return -1;
	}

	// It is important to set repeated to true to enable scrolling upwards
	distortionMap.setRepeated(true);
	// Setting smooth to true lets us use small maps even on larger images
	distortionMap.setSmooth(true);

	// Set texture to the shader
	shader.setUniform("currentTexture", sf::Shader::CurrentTexture);
	shader.setUniform("distortionMapTexture", distortionMap);

	// Shader parameters
	float distortionFactor = .05f;
	float riseFactor = .2f;


	// _________________
	// ::: Main loop :::

	sf::Clock timer;
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// Close the window if a key is pressed or if requested
			if (event.type == sf::Event::Closed) window.close();

			// If a key is pressed
			if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code)
				{
					// If escape is pressed, close the application
				case  sf::Keyboard::Escape: window.close(); break;

					// Process the up, down, left and right keys to modify parameters
				case sf::Keyboard::Up:     distortionFactor *= 2.f;    break;
				case sf::Keyboard::Down:    distortionFactor /= 2.f;    break;
				case sf::Keyboard::Left:    riseFactor *= 2.f;          break;
				case sf::Keyboard::Right:   riseFactor /= 2.f;          break;
				default: break;
				}
			}
		}

		// Set shader parameters
		shader.setUniform("time", timer.getElapsedTime().asSeconds());
		shader.setUniform("distortionFactor", distortionFactor);
		shader.setUniform("riseFactor", riseFactor);


		// Clear the window and apply grey background
		window.clear(sf::Color(127, 127, 127));

		// Draw the sprite and apply shader
		sprite.setPosition(window.getSize().x / 2., window.getSize().y / 2.);
		sprite.setScale(2, 2);
		window.draw(sprite, &shader);

		// Update display and wait for vsync
		window.display();
	}
	return 0;
}
