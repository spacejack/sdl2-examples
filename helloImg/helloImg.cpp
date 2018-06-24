#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Surface* image = NULL;

SDL_Surface* loadImage (std::string filename) {
	//The image that's loaded
	SDL_Surface* loadedImage = NULL;

	//The optimized image that will be used
	SDL_Surface* optimizedImage = NULL;

	loadedImage = IMG_Load(filename.c_str());
	if (loadedImage == NULL) {
		printf("Failed to load image: %s\n", SDL_GetError());
		return NULL;
	}

	//Create an optimized image
	optimizedImage = SDL_ConvertSurface(loadedImage, screen->format, 0);

	//Free the old image
	SDL_FreeSurface(loadedImage);

	return optimizedImage;
}

/** Blitter helper that fills in an SDL_Rect for us */
void blit (int x, int y, SDL_Surface* source, SDL_Surface* destination) {
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;
	SDL_BlitSurface(source, NULL, destination, &offset);
}

bool init() {
	// Initialize SDL subsystems
	// Use SDL_INIT_EVERYTHING for all
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return false;
	}

	int desiredFormats = IMG_INIT_PNG | IMG_INIT_JPG;
	printf("Desired format flags: %d\n", desiredFormats);
	int formats = IMG_Init(desiredFormats);
	printf("Available format flags: %d\n", formats);
	if ((formats & desiredFormats) != desiredFormats) {
		printf("Desired image formats unsupported\n");
		return false;
	}

	window = SDL_CreateWindow("SDL Image",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
	);
	if (window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	screen = SDL_GetWindowSurface(window);
	if (screen == NULL) {
		printf("Could not get window surface");
		return false;
	}

	SDL_FillRect(screen, NULL,
		SDL_MapRGB(screen->format, 0x00, 0x00, 0x00)
	);

	SDL_UpdateWindowSurface(window);

	//If everything initialized fine
	return true;
}

void cleanUp() {
	if (image != NULL) {
		SDL_FreeSurface(image);
	}
	IMG_Quit();
	SDL_Quit();
}

int main (int argc, char* args[]) {
	if (init() == false) {
		printf("Failed to initialize\n");
		return 1;
	}

	image = loadImage("ship.png");

	//If there was a problem in loading the image
	if (image == NULL) {
		cleanUp();
		return 1;
	}

	// Draw image to screen
	blit(
		(SCREEN_WIDTH - image->w) / 2, (SCREEN_HEIGHT - image->h) / 2,
		image, screen
	);
	SDL_UpdateWindowSurface(window);
	SDL_Delay(5000);

	cleanUp();
	return 0;
}
