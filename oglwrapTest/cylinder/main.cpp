#include <string>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;

static constexpr float CYL_HALF_HEIGHT = 0.5f;
static constexpr float CYL_RADIUS = 0.5f;
static constexpr int CYL_RINGS = 32;
static constexpr int CYL_SIDE_VERTS = (CYL_RINGS + 1) * 2;
static constexpr int CYL_CAP_VERTS = CYL_RINGS + 2;

SDL_Window* window = NULL;
SDL_GLContext context = NULL;

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

void initProgram (gl::Program& program, gl::VertexArray& vao, gl::ArrayBuffer& buffer) {
	{ // Define the cylinder geometry
		std::vector<glm::vec3> data;

		gl::Bind(vao);
		gl::Bind(buffer);

		// The side of the cylinder (to be rendered as a triangle strip)
		for (int i = 0; i <= CYL_RINGS; ++i) {
			float angle = i * 2*M_PI / CYL_RINGS;

			glm::vec3 top = {CYL_RADIUS*sin(angle), CYL_HALF_HEIGHT, CYL_RADIUS*cos(angle)};
			data.push_back(top); // position
			data.push_back(top - glm::vec3{0, top.y, 0}); // normal

			glm::vec3 bottom = {CYL_RADIUS*sin(angle), -CYL_HALF_HEIGHT, CYL_RADIUS*cos(angle)};
			data.push_back(bottom); // position
			data.push_back(bottom - glm::vec3{0, bottom.y, 0}); // normal
		}

		// The caps of the cylinder (to be rendered as a triangle fan)
		for (float y = -CYL_HALF_HEIGHT; y < CYL_HALF_HEIGHT + 1e-5; y += 2*CYL_HALF_HEIGHT) {
			glm::vec3 center = {0, y, 0};
			glm::vec3 normal = normalize(center);
			data.push_back(center); // position
			data.push_back(normal); // normal

			for (int i = 0; i <= CYL_RINGS; ++i) {
				float angle = i * 2*M_PI / CYL_RINGS;
				data.push_back({CYL_RADIUS*sin(angle), y, CYL_RADIUS*cos(angle)}); // position
				data.push_back(normal); // normal
			}
		}

		gl::VertexAttrib positions(gl::CubeShape::kPosition);
		positions.pointer(3, gl::DataType::kFloat, false, 2*sizeof(glm::vec3), (void*)0);
		positions.enable();

		gl::VertexAttrib normals(gl::CubeShape::kNormal);
		normals.pointer(3, gl::DataType::kFloat, false, 2*sizeof(glm::vec3), (void*)sizeof(glm::vec3));
		normals.enable();

		buffer.data(data);
		gl::Unbind(buffer);
		gl::Unbind(vao);
	}

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
	vs_source.set_source_file("example_shader.vert");
	gl::Shader vs(gl::kVertexShader, vs_source);

	gl::ShaderSource fs_source;
	fs_source.set_source(R"""(
		#version 330 core
		in vec3 normal;
		uniform vec3 color;
		out vec4 fragColor;
		void main() {
			vec3 lightPos = normalize(vec3(0.3, 1, 0.2));
			float diffuseLighting = 0.9*max(dot(lightPos, normalize(normal)), 0.0) + 0.1;
			fragColor = vec4(diffuseLighting * color, 1.0);
		})""");
	fs_source.set_source_file("example_shader.frag");
	gl::Shader fs(gl::kFragmentShader, fs_source);

	// Create a shader program
	program.attachShader(vs);
	program.attachShader(fs);
	program.link();
	gl::Use(program);

	(program | "inPos").bindLocation(gl::CubeShape::kPosition);
	(program | "inNormal").bindLocation(gl::CubeShape::kNormal);
}

void render (gl::CubeShape& cube, gl::Program& program, gl::VertexArray& vao) {
	gl::Clear().Color().Depth();
	float t = SDL_GetTicks() / 1000.0f;
	glm::mat4 camera_mat = glm::lookAt(
		2.5f * glm::vec3{sin(2 * t), 1.0f, cos(2 * t)},
		glm::vec3{0.0f, 0.0f, 0.0f},
		glm::vec3{0.0f, 1.0f, 0.0f}
	);
	glm::mat4 proj_mat = glm::perspectiveFov<float>(
		M_PI / 3.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.1, 100
	);

	gl::Use(program);

	// Cylinder
	{
		glm::mat4 model_mat = glm::translate(glm::mat4{1.0f}, glm::vec3{1, 0, 0});
		gl::Uniform<glm::mat4>(program, "mvp") = proj_mat * camera_mat * model_mat;
		gl::Uniform<glm::vec3>(program, "color") = glm::vec3{1.0, 0.0, 0.0};
		gl::Bind(vao);
		gl::DrawArrays(gl::PrimType::kTriangleStrip, 0, CYL_SIDE_VERTS);
		gl::DrawArrays(gl::PrimType::kTriangleFan, CYL_SIDE_VERTS, CYL_SIDE_VERTS + CYL_CAP_VERTS);
		gl::DrawArrays(gl::PrimType::kTriangleFan, CYL_SIDE_VERTS + CYL_CAP_VERTS, CYL_SIDE_VERTS + 2 * CYL_CAP_VERTS);
		gl::Unbind(vao);
	}

	// Cube
	{
		glm::mat4 model_mat = glm::translate(glm::mat4{1.0f}, glm::vec3{-1, 0, 0});
		gl::Uniform<glm::mat4>(program, "mvp") = proj_mat * camera_mat * model_mat;
		gl::Uniform<glm::vec3>(program, "color") = glm::vec3{1.0, 1.0, 0.0};
		cube.render();
	}

	gl::Unuse(program);
}

void mainLoop (gl::Program& program, gl::VertexArray& vao) {
	gl::CubeShape cube({gl::CubeShape::kPosition, gl::CubeShape::kNormal});
	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
		}
		render(cube, program, vao);
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
	gl::VertexArray vao;
	gl::ArrayBuffer buffer;
	initProgram(program, vao, buffer);
	printf("Initialized shader program\n");
	gl::Enable(gl::kDepthTest);
	gl::ClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	mainLoop(program, vao);
	cleanupSDL();
	printf("Quitting\n");
}
