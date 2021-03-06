// Dear ImGui: standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int float_to_char(int width, float f) {
  return (unsigned char) (f * (float) width);
}


int main(int, char **) {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;
  GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImFontAtlas *atlas = new ImFontAtlas();
  atlas->TexDesiredWidth = 128;
  atlas->Flags |= ImFontAtlasFlags_NoMouseCursors;
  ImGui::CreateContext(atlas);
  ImGuiIO &io = ImGui::GetIO();
  (void) io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
  ImGui::GetStyle().AntiAliasedLines = false;
  ImGui::GetStyle().AntiAliasedLinesUseTex = false;
  ImGui::GetStyle().AntiAliasedFill = false;

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL2_Init();

  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);

  ImFontConfig fontConfig{};
  fontConfig.SizePixels = 10;
  fontConfig.PixelSnapH = true;
  fontConfig.OversampleH = 1;
  fontConfig.OversampleV = 1;
  static const ImWchar ranges[] =
      {
          0x0020, 0x007E, // Basic Latin + Latin Supplement
          0,
      };
  fontConfig.GlyphRanges = &ranges[0];
  io.Fonts->AddFontFromFileTTF("imgui/misc/fonts/ProggyTiny.ttf", 10.0f, &fontConfig);

  // alright we made our font. Dump that shit.
  unsigned char *texData;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&texData, &width, &height);

  unsigned char *outData = new unsigned char[width * height / 2];
  memset(outData, 0, width * height / 2);

  constexpr int blockWidth = 8;
  constexpr int blockHeight = 8;
  const int xBlocks = width / blockWidth;
  const int yBlocks = height / blockHeight;

  for (int xBlock = 0; xBlock < xBlocks; xBlock++) {
    for (int yBlock = 0; yBlock < yBlocks; yBlock++) {
      int blockStartX = xBlock * blockWidth;
      int blockStartY = yBlock * blockHeight;
      int outStart = ((yBlock * xBlocks) + xBlock) * (blockWidth * blockHeight);
      // Ok now loop over the pixels
      for (int blockRelativeX = 0; blockRelativeX < blockWidth; blockRelativeX++) {
        for (int blockRelativeY = 0; blockRelativeY < blockHeight; blockRelativeY++) {

          int blockRelativeOffset = blockRelativeY * blockWidth + blockRelativeX;
          int globalOffset = (blockStartY + blockRelativeY) * width + (blockStartX + blockRelativeX);

          unsigned char pixel = (texData[globalOffset] >> 4) & 0xF;
//          unsigned char pixel = blockRelativeY * 64;
          int pixelPos = outStart + blockRelativeOffset;

          int bytePos = pixelPos / 2;
          if (pixelPos % 2 == 0) {
            outData[bytePos] |= pixel << 4;
          } else {
            outData[bytePos] |= pixel;
          }
        }
      }
      // Done with the block
    }
  }

  // ok build a header file

  std::ofstream out("font_atlas.hpp");

  out << "#pragma once\n\n";
  out << "#include \"imgui.h\"\n\n";
  out << "namespace FontAtlas {\n";

  out << "struct FontChar {\n";
  out << "bool Colored: 1;\n";
  out << "bool Visible: 1;\n";
  out << "unsigned char Codepoint;\n";
  out << "char AdvanceX;\n";
  out << "char X0;\n";
  out << "char Y0;\n";
  out << "char X1;\n";
  out << "char Y1;\n";
  out << "unsigned char U0;\n";
  out << "unsigned char V0;\n";
  out << "unsigned char U1;\n";
  out << "unsigned char V1;\n";
  out << "};\n\n";

  out << "struct CharVec4 {\n";
  out << "unsigned char W;\n";
  out << "unsigned char X;\n";
  out << "unsigned char Y;\n";
  out << "unsigned char Z;\n";
  out << "};\n\n";

  // font info
  ImFont *font = io.Fonts->Fonts[0];
  const ImFontConfig *configData = font->ConfigData;

  out << "const float INDEX_ADVANCE_X[] = {\n";
  for (int i = 0; i < font->IndexAdvanceX.Size; i++) {
    float v = font->IndexAdvanceX[i];
    out << v << ",";
  }
  out << "\n};\n";
  out << "constexpr float FALLBACK_ADVANCE_X = " << font->FallbackAdvanceX << ";\n";
  out << "constexpr int FONT_SIZE = " << font->FontSize << ";\n";

  out << "const ImWchar INDEX_LOOKUP[] = {\n";
  for (int i = 0; i < font->IndexLookup.Size; i++) {
    float v = font->IndexLookup[i];
    out << v << ",";
  }
  out << "\n};\n";
  out << "const FontChar FONT_GLYPHS[] = {\n";
  for (int i = 0; i < font->Glyphs.Size; i++) {
    ImFontGlyph *v = &font->Glyphs[i];
    out << "{";
    out << ".Colored=" << (v->Colored ? "true" : "false");
    out << ",.Visible=" << (v->Visible ? "true" : "false");
    out << ",.Codepoint=" << v->Codepoint;
    out << ",.AdvanceX=" << v->AdvanceX;
    out << ",.X0=" << v->X0 << ",.Y0=" << v->Y0;
    out << ",.X1=" << v->X1 << ",.Y1=" << v->Y1;
    out << ",.U0=" << float_to_char(atlas->TexWidth, v->U0) << ",.V0=" << float_to_char(atlas->TexHeight, v->V0);
    out << ",.U1=" << float_to_char(atlas->TexWidth, v->U1) << ",.V1=" << float_to_char(atlas->TexHeight, v->V1);
    out << "},\n";
  }
  out << "};\n";

  out << "constexpr float ASCENT = " << font->Ascent << ";\n";
  out << "constexpr float DESCENT = " << font->Descent << ";\n";
  out << "constexpr float MetricsTotalSurface = " << font->MetricsTotalSurface << ";\n";

  out << "const ImVec2 WhitePixel{" << atlas->TexUvWhitePixel.x << "," << atlas->TexUvWhitePixel.y << "};\n";

  out << "constexpr unsigned char Used4kPagesMap[(0xFFFF+1)/4096/8] = {\n";
  for (auto &v : font->Used4kPagesMap) {
    out << (int) v << ",";
  }
  out << "\n};\n";

//  ImVector<ImFontAtlasCustomRect> CustomRects;    // Rectangles for packing custom texture data into the atlas.
//  ImVector<ImFontConfig>      ConfigData;         // Configuration data

//  out << "const ImFontAtlasFlags AtlasFlags = " << atlas->Flags << ";\n";
  out << "const int PackedIdLines = " << atlas->PackIdLines << ";\n";
  out << "const int PackIdMouseCursors = " << atlas->PackIdMouseCursors << ";\n";

  out << "const CharVec4 TexUvLines[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1] = {\n";
  for (auto &v : atlas->TexUvLines) {
    out << "CharVec4{"
        << float_to_char(atlas->TexWidth, v.w) << ","
        << float_to_char(atlas->TexHeight, v.x) << ","
        << float_to_char(atlas->TexWidth, v.y) << ","
        << float_to_char(atlas->TexHeight, v.z) << "},\n";
  }
  out << "\n};\n";

  out << "const ImVec4 CustomRects[" << atlas->CustomRects.size() << "] = {\n";
  for (auto &v : atlas->CustomRects) {
    out << "{";
//    out << ".Width="<<v.Width;
//    out << ",.Height="<<v.Height;
    out << v.Width;
    out << "," << v.Height;
    out << "," << v.X;
    out << "," << v.Y;
//    out << ",.GlyphID="<<v.GlyphID;
//    out << ",.GlyphAdvanceX=" << v.GlyphAdvanceX;
//    out << ",.GlyphOffset={" << v.GlyphOffset.x << ","<< v.GlyphOffset.y << "}";
//    out << ",.Font="<<nullptr;
    out << "},\n";
  }
  out << "\n};\n";


  // font data
  out << "constexpr int ATLAS_W = " << width << ";\n";
  out << "constexpr int ATLAS_H = " << height << ";\n";

  out << "alignas(32) const unsigned char ATLAS_DATA[] = {";
  out << std::hex;
  constexpr int PER_LINE = 32;
  int currentLine = PER_LINE;
  for (int i = 0; i < width * height / 2; i++) {
    currentLine++;
    if (currentLine > PER_LINE) {
      out << "\n\t";
      currentLine = 0;
    }
    out << "0x" << (int) outData[i] << ",";
  }
  out << std::dec;
  out << "};\n\n";

  out << "}; //end namespace\n";
  out.close();


  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin(
          "Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

      if (ImGui::Button(
          "Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin("Another Window",
                   &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
    // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
    //GLint last_program;
    //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    //glUseProgram(0);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    //glUseProgram(last_program);

    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
