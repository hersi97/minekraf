#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_opengl.h"

int main(int argc, char *argv[])
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Error initializing SDL3: " << SDL_GetError() << "\n";
    return -1;
  }

  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  SDL_Window *window = SDL_CreateWindow("Hello Minekraf", 1280, 720, window_flags);

  if (!window) {
    std::cerr << "Error in SDL_CreateWindow: " << SDL_GetError() << "\n";
    return -2;
  }

  SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

  if (!gl_ctx) {
    std::cerr << "Error in SDL_GL_CreateContext: " << SDL_GetError() << "\n";
    return -3;
  }

  SDL_GL_MakeCurrent(window, gl_ctx);
  SDL_GL_SetSwapInterval(1);  // Vsync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplSDL3_InitForOpenGL(window, gl_ctx);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // main loop here
  bool run = true;
  bool show_demo_window = true;
  bool show_debug_log_window = true;
  while (run) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
    // inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite
    //   your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
    //   clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and
    //   hide them from your application based on those two flags.
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        run = false;
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
        run = false;
    }

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window) {
      ImGui::ShowDemoWindow(&show_demo_window);
    }

    if (show_debug_log_window) {
      ImGui::ShowDebugLogWindow(&show_debug_log_window);
    }

    ImGui::Render();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DestroyContext(gl_ctx);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
