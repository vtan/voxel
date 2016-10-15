#define GLM_FORCE_RADIANS

#include "camera.hpp"
#include "uniform.hpp"

#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <SDL.h>

struct SdlState
{
    SDL_Window *window;
};

SdlState initialize();
void cleanup(SdlState);

GLuint create_compiled_shader(GLenum, std::string);
GLuint create_linked_program(std::vector<GLenum>);
void create_bound_vao();
void create_bound_vbo();
std::string read_resource_from_file(std::string);

constexpr int screen_width = 1280;
constexpr int screen_height = 720;
constexpr GLfloat vertex_coords[] = {
    -1.0f, -1.0f, -3.0f,
    1.0f, -1.0f, -3.0f,
    0.0f,  1.0f, -3.0f,

    1.0f, 1.0f, -6.0f,
    -1.0f, 1.0f, -6.0f,
    0.0f,  -1.0f, -6.0f,
};
constexpr GLsizei vertex_count = 6;
constexpr GLint vertex_dim = 3;

int main()
{
    SdlState sdl_state = initialize();

    int gl_major_version;
    int gl_minor_version;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major_version);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor_version);
    printf("OpenGL %d.%d\n", gl_major_version, gl_minor_version);

    const GLuint vertex_shader_id = create_compiled_shader(
            GL_VERTEX_SHADER,
            read_resource_from_file("shader.vert"));
    const GLuint fragment_shader_id = create_compiled_shader(
            GL_FRAGMENT_SHADER,
            read_resource_from_file("shader.frag"));
    const GLuint program_id = create_linked_program(
            {vertex_shader_id, fragment_shader_id});
    glUseProgram(program_id);

    create_bound_vao();
    create_bound_vbo();

    constexpr float aspect_ratio = screen_width / (float) screen_height;
    Camera camera(aspect_ratio);
    camera.set_horizontal_angle(M_PI);

    Uniform<glm::mat4> model_to_clip(program_id, "modelToClip");
    model_to_clip.set(camera.calc_world_to_clip());

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    bool quit = false;
    while (!quit) {
        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_QUIT) {
                quit = true;
            } else if (sdl_event.type == SDL_KEYDOWN) {
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (sdl_event.key.keysym.sym == SDLK_KP_PLUS
                        || sdl_event.key.keysym.sym == SDLK_KP_MINUS) {
                    float sign = sdl_event.key.keysym.sym == SDLK_KP_PLUS
                        ? 1.f : -1.f;
                    camera.set_fov(camera.get_fov() + sign * M_PI / 36.f);
                    model_to_clip.set(camera.calc_world_to_clip());
                }
            } else if (sdl_event.type == SDL_MOUSEMOTION) {
                camera.set_horizontal_angle(
                        camera.get_horizontal_angle()
                            - sdl_event.motion.xrel * M_PI / 360.f);
                camera.set_vertical_angle(
                        camera.get_vertical_angle()
                            - sdl_event.motion.yrel * M_PI / 360.f);
                model_to_clip.set(camera.calc_world_to_clip());
            }
        }

        glClearColor(0.39f, 0.58f, 0.93f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, vertex_count);

        SDL_GL_SwapWindow(sdl_state.window);
    }

    cleanup(sdl_state);

    return 0;
}

SdlState initialize()
{
    SdlState sdl_state;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    sdl_state.window = SDL_CreateWindow(
            "voxel",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            screen_width, screen_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_GL_CreateContext(sdl_state.window);
    glewExperimental = GL_TRUE;
    glewInit();

    return sdl_state;
}

void cleanup(SdlState sdl_state)
{
    SDL_DestroyWindow(sdl_state.window);
    SDL_Quit();
}

GLuint create_compiled_shader(
        const GLenum shader_type, const std::string source)
{
    static constexpr GLsizei info_log_max_length = 4096;

    const GLuint shader_id = glCreateShader(shader_type);
    const GLchar* source_cstr = source.c_str();
    glShaderSource(shader_id, 1, &source_cstr, nullptr);
    glCompileShader(shader_id);

    GLchar info_log_cstr[info_log_max_length];
    glGetShaderInfoLog(shader_id, info_log_max_length, nullptr, info_log_cstr);
    const std::string info_log = info_log_cstr;

    if (info_log.empty()) {
        return shader_id;
    } else {
        throw std::runtime_error(info_log);
    }
}

GLuint create_linked_program(const std::vector<GLenum> shader_ids)
{
    static constexpr GLsizei info_log_max_length = 4096;

    const GLuint program_id = glCreateProgram();
    for (const GLuint shader_id : shader_ids) {
        glAttachShader(program_id, shader_id);
    }
    glLinkProgram(program_id);

    GLchar info_log_cstr[info_log_max_length];
    glGetProgramInfoLog(
            program_id, info_log_max_length, nullptr, info_log_cstr);
    const std::string info_log = info_log_cstr;

    if (info_log.empty()) {
        return program_id;
    } else {
        throw std::runtime_error(info_log);
    }
}

void create_bound_vao()
{
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
}

void create_bound_vbo()
{
    static constexpr GLuint attr_index = 0;

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(vertex_coords), vertex_coords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glVertexAttribPointer(
            attr_index, vertex_dim, GL_FLOAT, GL_FALSE, 0, nullptr);
}

std::string read_resource_from_file(const std::string resource_name)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open("res/" + resource_name);

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}
