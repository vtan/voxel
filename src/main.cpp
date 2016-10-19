#define GLM_FORCE_RADIANS

#include "camera.hpp"
#include "mesh.hpp"
#include "mesh_builder.hpp"
#include "uniform.hpp"
#include "volume.hpp"
#include "voxel.hpp"

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

Volume<Voxel> create_volume(size_t z, size_t y, size_t x);

GLuint create_compiled_shader(GLenum, std::string);
GLuint create_linked_program(std::vector<GLenum>);
std::string read_resource_from_file(std::string);

constexpr int screen_width = 1280;
constexpr int screen_height = 720;

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

    const Volume<Voxel> volume = create_volume(10, 10, 10);
    Mesh mesh;
    MeshBuilder mesh_builder;
    mesh.build_vao(mesh_builder.build(volume));

    constexpr float aspect_ratio = screen_width / (float) screen_height;
    Camera camera(aspect_ratio);

    Uniform<glm::mat4> model_to_clip(program_id, "modelToClip");
    model_to_clip.set(camera.calc_world_to_clip());

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    float velocity_forward = 0.f;
    float velocity_right = 0.f;

    bool quit = false;
    while (!quit) {
        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_QUIT) {
                quit = true;
            } else if (sdl_event.type == SDL_KEYDOWN) {
                if (sdl_event.key.repeat) { continue; }

                const auto sc = sdl_event.key.keysym.scancode;
                if (sc == SDL_SCANCODE_ESCAPE) {
                    quit = true;
                } else if (sc == SDL_SCANCODE_KP_PLUS
                        || sc == SDL_SCANCODE_KP_MINUS) {
                    float sign = sc == SDL_SCANCODE_KP_PLUS ? 1.f : -1.f;
                    camera.set_fov(camera.get_fov() + sign * M_PI / 36.f);
                    model_to_clip.set(camera.calc_world_to_clip());
                } else if (sc == SDL_SCANCODE_W || sc == SDL_SCANCODE_UP) {
                    velocity_forward = 0.1f;
                } else if (sc == SDL_SCANCODE_S || sc == SDL_SCANCODE_DOWN) {
                    velocity_forward = -0.1f;
                } else if (sc == SDL_SCANCODE_A || sc == SDL_SCANCODE_LEFT) {
                    velocity_right = -0.1f;
                } else if (sc == SDL_SCANCODE_D || sc == SDL_SCANCODE_RIGHT) {
                    velocity_right = 0.1f;
                }
            } else if (sdl_event.type == SDL_KEYUP) {
                const auto sc = sdl_event.key.keysym.scancode;
                if (sc == SDL_SCANCODE_W || sc == SDL_SCANCODE_UP
                        || sc == SDL_SCANCODE_S || sc == SDL_SCANCODE_DOWN) {
                    velocity_forward = 0.f;
                } else if (sc == SDL_SCANCODE_A || sc == SDL_SCANCODE_LEFT
                        || sc == SDL_SCANCODE_D || sc == SDL_SCANCODE_RIGHT) {
                    velocity_right = 0.f;
                }
            } else if (sdl_event.type == SDL_MOUSEMOTION) {
                const float hor_angle = camera.get_horizontal_angle()
                    - sdl_event.motion.xrel * M_PI / 360.f;
                const float ver_angle = camera.get_vertical_angle()
                    - sdl_event.motion.yrel * M_PI / 360.f;
                camera.look(hor_angle, ver_angle);
                model_to_clip.set(camera.calc_world_to_clip());
            }
        }

        if (velocity_forward != 0.f || velocity_right != 0.f) {
            camera.move(velocity_forward);
            camera.move_right(velocity_right);
            model_to_clip.set(camera.calc_world_to_clip());
        }

        glClearColor(0.39f, 0.58f, 0.93f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mesh.draw();

        SDL_GL_SwapWindow(sdl_state.window);
    }

    cleanup(sdl_state);

    return 0;
}

Volume<Voxel> create_volume(
        const size_t z_size, const size_t y_size, const size_t x_size)
{
    Volume<Voxel> volume(x_size, y_size, z_size, Voxel::empty);

    const glm::vec3 center(
            x_size / 2 - 0.5f, y_size / 2 - 0.5f, z_size / 2 - 0.5f);
    const float radius = x_size / 2 - 1;

    volume.for_each_voxel_in_border(1, 1, 1, [&](auto x, auto y, auto z) {
        const glm::vec3 pos(x, y, z);
        if (glm::distance(pos, center) <= radius) {
            volume.at(x, y, z) = Voxel::solid;
        }
    });

    return volume;
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

std::string read_resource_from_file(const std::string resource_name)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open("res/" + resource_name);

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}
