#include <string>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/rectangle_shape.h>

SDL_Window* window = NULL;
SDL_GLContext context;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
bool quit = false;

bool initSDL() {
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	} else {
		//Use OpenGL 3.1 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Create window
		window = SDL_CreateWindow(
			"SDL OpenGL Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
		);
		if (window == NULL) {
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			return false;
		} else {
			//Create context
			context = SDL_GL_CreateContext(window);
			if (context == NULL) {
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				return false;
			} else {
				//Initialize GLEW
				glewExperimental = GL_TRUE;
				GLenum glewError = glewInit();
				if (glewError != GLEW_OK) {
					printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
				}

				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0) {
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}
			}
		}
	}

	return true;
}

void cleanupSDL() {
	//Destroy window
	SDL_DestroyWindow(window);
	window = NULL;
	//Quit SDL subsystems
	SDL_Quit();
}

void initShader (gl::Program& program) {
	// Create a vertex shader
	gl::ShaderSource vs_source;
	vs_source.set_source(R"""(
		#version 330 core
		in vec2 pos;
		void main() {
			// Shrink the full screen rectangle to only half size
			gl_Position = vec4(0.5 * pos.xy, 0, 1);
		})""");
	// Give a name for the shader (will be displayed in diagnostic messages)
	vs_source.set_source_file("example_shader.vert");
	gl::Shader vs(gl::kVertexShader, vs_source);

	// Create a fragment shader
	gl::ShaderSource fs_source;
	fs_source.set_source(R"""(
		#version 330 core
		out vec4 fragColor;
		void main() {
			fragColor = vec4(0.3, 0.4, 0.7, 1.0);
		})""");
	// Give a name for the shader (will be displayed in diagnostic messages)
	fs_source.set_source_file("example_shader.frag");
	gl::Shader fs(gl::kFragmentShader, fs_source);

	// Create the shader program
	program.attachShader(vs);
	program.attachShader(fs);
	program.link();
	gl::Use(program);

	// Bind the attribute position to the location that the RectangleShape uses
	// (Both use attribute 0 by default for position, so this call isn't necessary)
	(program | "pos").bindLocation(gl::RectangleShape::kPosition);

	// Set the clear color to grey
	gl::ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void render (gl::RectangleShape& rectangle) {
	rectangle.render();
}

void mainLoop() {
	gl::RectangleShape rectangle;
	SDL_Event e;

	//Enable text input
	//SDL_StartTextInput();

	//While application is running
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			//User requests quit
			if (e.type == SDL_QUIT) {
				quit = true;
				break;
			}
			//Handle keypress with current mouse position
			// else if (e.type == SDL_TEXTINPUT) {
			// 	int x = 0, y = 0;
			// 	SDL_GetMouseState(&x, &y);
			// 	handleKeys(e.text.text[0], x, y);
			// }
		}

		//Render quad
		render(rectangle);

		//Update screen
		SDL_GL_SwapWindow(window);
	}
}

int main (int argc, char* args[]) {
	if (!initSDL()) {
		printf("Failed to init SDL.\n");
		return 1;
	}
	printf("Initialized SDL\n");
	gl::Program program;
	initShader(program);
	printf("Initialized shader\n");
	mainLoop();
	cleanupSDL();
	printf("Quitting\n");
}
