#include <string>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

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

void initProgram (gl::Program& program) {
    gl::ShaderSource vs_source;
    vs_source.set_source(R"""(
		#version 330 core
		in vec4 inPos;
		in vec3 inNormal;
		uniform mat4 mvp;
		out vec3 normal;
		void main() {
			normal = inNormal;
			gl_Position = mvp * inPos;
		})""");
    vs_source.set_source_file("cube.vert");
    gl::Shader vs(gl::kVertexShader, vs_source);

    gl::ShaderSource fs_source;
    fs_source.set_source(R"""(
		#version 330 core
		in vec3 normal;
		out vec4 fragColor;
		void main() {
			vec3 lightPos = normalize(vec3(0.3, 1, 0.2));
			float diffuseLighting = max(dot(lightPos, normalize(normal)), 0.0);
			vec3 color = vec3(0.1, 0.3, 0.8);
			fragColor = vec4(diffuseLighting * color, 1.0);
		})""");
    fs_source.set_source_file("cube.frag");
    gl::Shader fs(gl::kFragmentShader, fs_source);

    // Create a shader program
    program.attachShader(vs);
    program.attachShader(fs);
    program.link();
    gl::Use(program);

    // Bind the attribute locations
    (program | "inPos").bindLocation(gl::CubeShape::kPosition);
    (program | "inNormal").bindLocation(gl::CubeShape::kNormal);
}

void render (gl::CubeShape& cube, gl::Program& program) {
	gl::Clear().Color().Depth();
	float t = SDL_GetTicks() / 1000.0f;
	glm::mat4 camera_mat = glm::lookAt(
		1.5f * glm::vec3{sin(t), 1.0f, cos(t)},
		glm::vec3{0.0f, 0.0f, 0.0f},
		glm::vec3{0.0f, 1.0f, 0.0f}
	);
	glm::mat4 proj_mat = glm::perspectiveFov<float>(
		M_PI / 3.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.1, 100
	);
	gl::Uniform<glm::mat4>(program, "mvp") = proj_mat * camera_mat;
	cube.render();
}

void mainLoop (gl::Program& program) {
	gl::CubeShape cube({gl::CubeShape::kPosition, gl::CubeShape::kNormal});
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
		render(cube, program);

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
	initProgram(program);
	printf("Initialized shader program\n");
	gl::Enable(gl::kDepthTest);
	gl::ClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	mainLoop(program);
	cleanupSDL();
	printf("Quitting\n");
}
