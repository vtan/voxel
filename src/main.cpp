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

enum class Voxel : uint8_t
{
    empty,
    solid,
};

SdlState initialize();
void cleanup(SdlState);

std::vector<Voxel> create_volume(size_t z, size_t y, size_t x);
std::vector<glm::vec3> create_mesh_from_volume(
        std::vector<Voxel>, size_t z, size_t y);

GLuint create_compiled_shader(GLenum, std::string);
GLuint create_linked_program(std::vector<GLenum>);
void create_bound_vao();
void create_bound_vbo(std::vector<glm::vec3> vertex_positions);
std::string read_resource_from_file(std::string);

constexpr int screen_width = 1280;
constexpr int screen_height = 720;
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

    const auto volume = create_volume(10, 10, 10);
    const auto mesh = create_mesh_from_volume(volume, 10, 10);

    create_bound_vao();
    create_bound_vbo(mesh);

    constexpr float aspect_ratio = screen_width / (float) screen_height;
    Camera camera(aspect_ratio);

    Uniform<glm::mat4> model_to_clip(program_id, "modelToClip");
    model_to_clip.set(camera.calc_world_to_clip());

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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

        glDrawArrays(GL_TRIANGLES, 0, mesh.size());

        SDL_GL_SwapWindow(sdl_state.window);
    }

    cleanup(sdl_state);

    return 0;
}

std::vector<Voxel> create_volume(
        const size_t z_size, const size_t y_size, const size_t x_size)
{
    std::vector<Voxel> voxels(z_size * y_size * x_size, Voxel::empty);

    const size_t xy_size = x_size * y_size;
    const glm::vec3 center(
            x_size / 2 - 0.5f, y_size / 2 - 0.5f, z_size / 2 - 0.5f);
    const float radius = x_size / 2 - 1;

    for (size_t z = 1; z < z_size - 1; ++z) {
        for (size_t y = 1; y < y_size - 1; ++y) {
            for (size_t x = 1; x < x_size - 1; ++x) {
                const glm::vec3 pos(x, y, z);
                if (glm::distance(pos, center) <= radius) {
                    voxels[z * xy_size + y * x_size + x] = Voxel::solid;
                }
            }
        }
    }

    return voxels;
}

// The border voxels are considered neighbors and are not included in the mesh.
// TODO place at 0,0,0 not 1,1,1
// TODO place towards -z?
// TODO check winding order
std::vector<glm::vec3> create_mesh_from_volume(
        const std::vector<Voxel> voxels,
        const size_t z_size,
        const size_t y_size)
{
    std::vector<glm::vec3> vertex_positions;

    const size_t x_size = voxels.size() / z_size / y_size;
    const size_t xy_size = x_size * y_size;
    for (size_t z = 1; z < z_size - 1; ++z) {
        for (size_t y = 1; y < y_size - 1; ++y) {
            for (size_t x = 1; x < x_size - 1; ++x) {
                const Voxel current = voxels[z * xy_size + y * x_size + x];
                if (current != Voxel::empty) {
                    const Voxel left =
                        voxels[z * xy_size + y * x_size + (x - 1)];
                    if (left == Voxel::empty) {
                        vertex_positions.insert(vertex_positions.end(), {
                                glm::vec3(x, y, z),
                                glm::vec3(x, y, z + 1),
                                glm::vec3(x, y + 1, z),
                                glm::vec3(x, y, z + 1),
                                glm::vec3(x, y + 1, z + 1),
                                glm::vec3(x, y + 1, z),
                        });
                    }

                    const Voxel right =
                        voxels[z * xy_size + y * x_size + (x + 1)];
                    if (right == Voxel::empty) {
                        vertex_positions.insert(vertex_positions.end(), {
                                glm::vec3(x + 1, y, z),
                                glm::vec3(x + 1, y, z + 1),
                                glm::vec3(x + 1, y + 1, z),
                                glm::vec3(x + 1, y, z + 1),
                                glm::vec3(x + 1, y + 1, z),
                                glm::vec3(x + 1, y + 1, z + 1),
                        });
                    }

                    const Voxel below =
                        voxels[z * xy_size + (y - 1) * x_size + x];
                    if (below == Voxel::empty) {
                        vertex_positions.insert(vertex_positions.end(), {
                                glm::vec3(x, y, z),
                                glm::vec3(x + 1, y, z),
                                glm::vec3(x, y, z + 1),
                                glm::vec3(x + 1, y, z),
                                glm::vec3(x + 1, y, z + 1),
                                glm::vec3(x, y, z + 1),
                        });
                    }

                    const Voxel above =
                        voxels[z * xy_size + (y + 1) * x_size + x];
                    if (above == Voxel::empty) {
                        vertex_positions.insert(vertex_positions.end(), {
                                glm::vec3(x, y + 1, z),
                                glm::vec3(x, y + 1, z + 1),
                                glm::vec3(x + 1, y + 1, z),
                                glm::vec3(x + 1, y + 1, z),
                                glm::vec3(x, y + 1, z + 1),
                                glm::vec3(x + 1, y + 1, z + 1),
                        });
                    }

                    const Voxel front =
                        voxels[(z - 1) * xy_size + y * x_size + x];
                    if (front == Voxel::empty) {
                        vertex_positions.insert(vertex_positions.end(), {
                                glm::vec3(x, y, z),
                                glm::vec3(x, y + 1, z),
                                glm::vec3(x + 1, y, z),
                                glm::vec3(x, y + 1, z),
                                glm::vec3(x + 1, y + 1, z),
                                glm::vec3(x + 1, y, z),
                        });
                    }

                    const Voxel back =
                        voxels[(z + 1) * xy_size + y * x_size + x];
                    if (back == Voxel::empty) {
                        vertex_positions.insert(vertex_positions.end(), {
                                glm::vec3(x, y, z + 1),
                                glm::vec3(x + 1, y, z + 1),
                                glm::vec3(x, y + 1, z + 1),
                                glm::vec3(x, y + 1, z + 1),
                                glm::vec3(x + 1, y, z + 1),
                                glm::vec3(x + 1, y + 1, z + 1),
                        });
                    }
                }
            }
        }
    }

    return vertex_positions;
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

void create_bound_vbo(const std::vector<glm::vec3> vertex_positions)
{
    static constexpr GLuint attr_index = 0;

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(
            GL_ARRAY_BUFFER,
            vertex_positions.size() * sizeof(glm::vec3),
            vertex_positions.data(),
            GL_STATIC_DRAW);
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
