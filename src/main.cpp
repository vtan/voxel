#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL.h>

struct SdlState
{
    SDL_Window *window;
};

SdlState initialize();
void cleanup(SdlState);

int main()
{
    SdlState sdl_state = initialize();

    int gl_major_version;
    int gl_minor_version;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major_version);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor_version);
    printf("OpenGL %d.%d\n", gl_major_version, gl_minor_version);

    bool quit = false;
    while (!quit) {
        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_QUIT
                    || sdl_event.type == SDL_KEYDOWN) {
                quit = true;
            }
        }

        glClearColor(0.39f, 0.58f, 0.93f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    sdl_state.window = SDL_CreateWindow(
            "voxel",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_GL_CreateContext(sdl_state.window);
    glewInit();

    return sdl_state;
}

void cleanup(SdlState sdl_state)
{
    SDL_DestroyWindow(sdl_state.window);
    SDL_Quit();
}
