/*
MIT License

Copyright (c) 2019 Light Transport Entertainment Inc. And many contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "gui_util.hh"

// Need to access some OpenGL functions in there:
#include "gl_util.hh"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

// Include embeded data.
// These files defines each an array of bytes and a size.
// They have been automatically generated from tools in the
// fonts and icons directory.
#include "gltf-insight-128.png.inc.hh"
#include "gltf-insight-16.png.inc.hh"
#include "gltf-insight-256.png.inc.hh"
#include "gltf-insight-32.png.inc.hh"
#include "gltf-insight-48.png.inc.hh"
#include "gltf-insight-64.png.inc.hh"
#include "gltf-insight-96.png.inc.hh"
#include "ionicons_embed.inc.h"
//#include "roboto_embed.inc.h"
#include "roboto_light_embed.inc.h"
#include "roboto_mono_embed.inc.h"

// image loader
#include "stb_image.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#if __has_warning("-Wc++2a-compat")
#pragma clang diagnostic ignored "-Wc++2a-compat"
#endif

#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void gui_new_frame() {
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();
}

void gl_new_frame(GLFWwindow* window, ImVec4 clear_color, int& display_w,
                  int& display_h) {
  // Rendering
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
}

void gl_gui_end_frame(GLFWwindow* window) {
  glUseProgram(0);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window);
  glFlush();

  static int frameCount = 0;
  static double currentTime = glfwGetTime();
  static double previousTime = currentTime;
  static char title[256];

  frameCount++;
  currentTime = glfwGetTime();
  const auto deltaTime = currentTime - previousTime;
  if (deltaTime >= 1.0) {
    sprintf(title, "glTF Insight GUI [%dFPS]", frameCount);
    glfwSetWindowTitle(window, title);
    frameCount = 0;
    previousTime = currentTime;
  }
}

bool ImGuiCombo(const char* label, int* current_item,
                const std::vector<std::string>& items) {
  return ImGui::Combo(
      label, current_item,
      [](void* data, int idx_i, const char** out_text) {
        size_t idx = static_cast<size_t>(idx_i);
        const std::vector<std::string>* str_vec =
            reinterpret_cast<std::vector<std::string>*>(data);
        if (idx_i < 0 || str_vec->size() <= idx) {
          return false;
        }
        *out_text = str_vec->at(idx).c_str();
        return true;
      },
      reinterpret_cast<void*>(const_cast<std::vector<std::string>*>(&items)),
      static_cast<int>(items.size()), static_cast<int>(items.size()));
}

void asset_images_window(const std::vector<GLuint>& textures, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("glTF Images", open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Number of textures [%zu]", textures.size());
    ImGui::BeginChild("##ScrollableRegion0", ImVec2(256, 286), false,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar);
    for (int i = 0; i < int(textures.size()); ++i) {
      auto& texture = textures[size_t(i)];
      std::string name = "texture [" + std::to_string(i) + "]";
      if (ImGui::CollapsingHeader(name.c_str())) {
        ImGui::Image(ImTextureID(size_t(texture)), ImVec2(256.f, 256.f));
      }
    }
    ImGui::EndChild();
  }
  ImGui::End();
}

static void error_callback(int error, const char* description) {
  std::cerr << "GLFW Error : " << error << ", " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods) {
  (void)scancode;

  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    return;
  }

  if (key == GLFW_KEY_Q && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void describe_node_topology_in_imgui_tree(const tinygltf::Model& model,
                                          int node_index) {
  std::string node_desc = "node [" + std::to_string(node_index) + "]";
  if (ImGui::TreeNode(node_desc.c_str())) {
    const auto& node = model.nodes[size_t(node_index)];
    if (!node.name.empty()) ImGui::Text("name [%s]", node.name.c_str());

    for (auto child : node.children) {
      describe_node_topology_in_imgui_tree(model, child);
    }

    ImGui::TreePop();
  }
}

void model_info_window(const tinygltf::Model& model, bool* open) {
  if (open && !*open) return;
  // TODO also cache info found here
  if (ImGui::Begin("Model information", open)) {
    const auto main_node_index = find_main_mesh_node(model);
    if (main_node_index < 0) {
      ImGui::Text("Could not find a node with a mesh in your glTF file!");
      return;
    }

    ImGui::Text("Main node with a mesh [%d]", main_node_index);
    const auto& main_node = model.nodes[size_t(main_node_index)];

    if (main_node.skin >= 0) {
      // mesh.weights
      ImGui::Text("Main mesh uses skin [%d]", main_node.skin);
      const auto& skin = model.skins[size_t(main_node.skin)];
      ImGui::Text("Skin [%d] skeleton root node [%d]", main_node.skin,
                  skin.skeleton);
      ImGui::Text("Skin joint count [%zu]", skin.joints.size());
      // if (ImGui::CollapsingHeader("Skeleton topology"))
      //  describe_node_topology_in_imgui_tree(model, skin.skeleton);
    }

    // TODO check if the found node with a mesh has morph targets, and
    // describe them in this window here
  }
  ImGui::End();
}

// Need access to mesh type.
// TODO create mesh.hh and extract this definition
#include "insight-app.hh"
void mesh_display_window(std::vector<gltf_insight::mesh>& meshes, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("Mesh visibility", open)) {
    for (auto& mesh : meshes)
      ImGui::Checkbox(mesh.name.c_str(), &mesh.displayed);
  }
  ImGui::End();
}

#if 0  // I'm keeping this for the sake of the ImGui table here, I'm going to
       // reuse this at some point
void skinning_data_window(
    const std::vector<std::vector<float>>& weights,
    const std::vector<std::vector<unsigned short>>& joints) {
  ImGui::Begin("Skinning data", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  static int submesh_id = 0;
  ImGui::InputInt("Submesh #", &submesh_id, 1, 1);
  submesh_id = glm::clamp<int>(submesh_id, 0, int(weights.size()) - 1);
  ImGui::Text("Submesh[%zu]", size_t(submesh_id));
  ImGui::BeginChild("##scrollable_data_region", ImVec2(600, 800), false,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar);
  const auto vertex_count = weights[size_t(submesh_id)].size() / 4;
  assert(vertex_count == joints[size_t(submesh_id)].size() / 4);

  const auto& weight = weights[size_t(submesh_id)];
  const auto& joint = joints[size_t(submesh_id)];

  ImGui::Columns(5);
  ImGui::Text("Vertex Index");
  ImGui::NextColumn();
  ImGui::Text("0");
  ImGui::NextColumn();
  ImGui::Text("1");
  ImGui::NextColumn();
  ImGui::Text("2");
  ImGui::NextColumn();
  ImGui::Text("3");
  ImGui::NextColumn();
  ImGui::Separator();

  for (size_t i = 0; i < vertex_count; ++i) {
    ImGui::Text("%zu", i);
    ImGui::NextColumn();
    ImGui::Text("%f * %d", double(weight[i * 4]), joint[i * 4]);
    ImGui::NextColumn();
    ImGui::Text("%f * %d", double(weight[i * 4 + 1]), joint[i * 4 + 1]);
    ImGui::NextColumn();
    ImGui::Text("%f * %d", double(weight[i * 4 + 2]), joint[i * 4 + 2]);
    ImGui::NextColumn();
    ImGui::Text("%f * %d", double(weight[i * 4 + 3]), joint[i * 4 + 3]);
    ImGui::NextColumn();
    ImGui::Separator();
  }
  ImGui::Columns();
  ImGui::EndChild();
  ImGui::End();
}
#endif

void morph_target_window(gltf_node& mesh_skeleton_graph, std::vector<gltf_insight::mesh>& meshes,
                         bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("Morph Target blend weights", open)) {
    std::vector<std::string> meshesNames;
    for (const auto& mesh : meshes) meshesNames.push_back(mesh.name);
    static int currentItem = 0;
    if (!(currentItem < meshesNames.size())) currentItem = 0;

    ImGuiCombo("Mesh", &currentItem, meshesNames);

    for (int targetIndex = 0; targetIndex < meshes[currentItem].nb_morph_targets; ++targetIndex) {
      std::string name;

      if (mesh_skeleton_graph.pose.target_names.empty() ||
          mesh_skeleton_graph.pose.target_names[size_t(targetIndex)].empty())
        name = "Morph Target [" + std::to_string(targetIndex) + "]";
      else
        name = mesh_skeleton_graph.pose.target_names[size_t(targetIndex)];

      ImGui::SliderFloat(name.c_str(),
                         &mesh_skeleton_graph.pose.blend_weights[size_t(targetIndex)], 0,
                         1, "%f");
      mesh_skeleton_graph.pose.blend_weights[size_t(targetIndex)] = glm::clamp(
          mesh_skeleton_graph.pose.blend_weights[size_t(targetIndex)], 0.f, 1.f);
    }
  }
  ImGui::End();
}

static void load_and_set_window_icons(GLFWwindow*& window) {
  int x, y, c;
  std::array<GLFWimage, 3> images;

  // https://www.glfw.org/docs/3.2/group__window.html#gadd7ccd39fe7a7d1f0904666ae5932dc5

  // We are loading only a few sizes of the availalbe icons

  images[2].pixels = stbi_load_from_memory(
      gltf_insight_48_png, int(gltf_insight_48_png_len), &x, &y, &c, 4);
  assert(x == 48 && y == 48 && "loaded data should be 48px icon");
  images[2].width = x;
  images[2].height = y;

  images[1].pixels = stbi_load_from_memory(
      gltf_insight_32_png, int(gltf_insight_32_png_len), &x, &y, &c, 4);
  assert(x == 32 && y == 32 && "loaded data should be 32px icon");
  images[1].width = x;
  images[1].height = y;

  images[0].pixels = stbi_load_from_memory(
      gltf_insight_16_png, int(gltf_insight_16_png_len), &x, &y, &c, 4);
  assert(x == 16 && y == 16 && "loaded data should be 16px icon");
  images[0].width = x;
  images[0].height = y;

  // glfw actually copies these images
  glfwSetWindowIcon(window, 3, images.data());

  for (auto image : images) stbi_image_free(image.pixels);
}

void initialize_glfw_opengl_window(GLFWwindow*& window) {
  // Setup window
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

#ifdef _DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

#ifdef __EMSCRIPTEN__
  // Create a GLES 3.0 context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_CONTEXT_CREATION_API , GLFW_EGL_CONTEXT_API);
  glfwWindowHint(GLFW_SAMPLES, 16);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

#else
  // We need GL 3.3 context since we use `inverse()` in the shader.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 16);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

  // still want compatibility with old OpenGL
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE);  // It looks this is important on macOS.
#else
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
#endif

#endif
  window = glfwCreateWindow(1600, 900, "glTF Insight GUI", nullptr, nullptr);
  glfwMakeContextCurrent(window);

#ifdef __APPLE__
  glfwSwapInterval(1);  // Enable vsync
#else
  glfwSwapInterval(0);
#endif
  glfwSetKeyCallback(window, key_callback);

// glad must be called after glfwMakeContextCurrent()
#ifndef __EMSCRIPTEN__
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "Failed to load OpenGL functions with gladLoadGL\n";
    exit(EXIT_FAILURE);
  }

  std::cout << "OpenGL " << GLVersion.major << '.' << GLVersion.minor << '\n';

  if (!((GLVersion.major >= 3) && (GLVersion.minor >= 3)) &&
      (GLVersion.major <= 3)) {
    std::cerr << "OpenGL 3.3 or later is not available." << std::endl;
    exit(EXIT_FAILURE);
  }
#endif

  std::cout << "GL_VENDOR : " << glGetString(GL_VENDOR) << "\n";
  std::cout << "GL_RENDERER : " << glGetString(GL_RENDERER) << "\n";
  std::cout << "GL_VERSION : " << glGetString(GL_VERSION) << "\n";

#ifdef _DEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(glDebugOutput), nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                        GL_TRUE);
#endif

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glFrontFace(GL_CW);

  // load a window icon
  load_and_set_window_icons(window);
}

void initialize_imgui(GLFWwindow* window) {
  // Setup Dear ImGui context
  ImGui::CreateContext();
  auto& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  const float default_font_scale = 16.f;
  ImFontConfig roboto_config;
  strcpy(roboto_config.Name, "Roboto");
  roboto_config.SizePixels = default_font_scale;
#if defined(__APPLE__)
  // Assuem retina display. 2 is suffice
  roboto_config.OversampleH = 2;
  roboto_config.OversampleV = 2;
#else
  // 2 is a bit blurry on Windows. 4~8 gives nicer anti aliasing
  roboto_config.OversampleH = 6;
  roboto_config.OversampleV = 6;
#endif

#if 0
  io.Fonts->AddFontFromMemoryCompressedTTF(roboto_compressed_data,
                                           roboto_compressed_size,
                                           default_font_scale, &roboto_config);
#else
  io.Fonts->AddFontFromMemoryCompressedTTF(roboto_light_compressed_data,
                                           roboto_light_compressed_size,
                                           default_font_scale, &roboto_config);

#endif

  ImFontConfig ionicons_config;
  ionicons_config.MergeMode = true;
  ionicons_config.GlyphMinAdvanceX = default_font_scale;
  ionicons_config.OversampleH = 1;
  ionicons_config.OversampleV = 1;
  static const ImWchar icon_ranges[] = {ICON_MIN_II, ICON_MAX_II, 0};
  io.Fonts->AddFontFromMemoryCompressedTTF(
      ionicons_compressed_data, ionicons_compressed_size, default_font_scale,
      &ionicons_config, icon_ranges);

  ImFontConfig roboto_mono_config;
  strcpy(roboto_mono_config.Name, "Roboto Mono");
  roboto_mono_config.SizePixels = default_font_scale;
  roboto_mono_config.OversampleH = 2;
  roboto_mono_config.OversampleV = 2;
  io.Fonts->AddFontFromMemoryCompressedTTF(
      roboto_mono_compressed_data, roboto_mono_compressed_size,
      default_font_scale, &roboto_mono_config);

  ImGuiFileDialog::fileLabel = ICON_II_ANDROID_DOCUMENT;
  ImGuiFileDialog::dirLabel = ICON_II_ANDROID_FOLDER;
  ImGuiFileDialog::linkLabel = ICON_II_ANDROID_ARROW_FORWARD;

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__  // Just Web for now (Android in the future)
  // ESSL 3.00
  ImGui_ImplOpenGL3_Init("#version 300 es");
#else  // All destktop platforms
  // GLSL 3.30
  ImGui_ImplOpenGL3_Init("#version 330");
#endif

#ifndef FORCE_DEFAULT_STYLE
  // Setup Style
  // I took that nice dark color scheme from Derydoca as a base:
  // https://github.com/ocornut/imgui/issues/707?ts=4#issuecomment-463758243
  ImGui::StyleColorsDark();
  ImGui::GetStyle().WindowRounding = 1.0f;
  ImGui::GetStyle().ChildRounding = 1.0f;
  ImGui::GetStyle().FrameRounding = 1.0f;
  ImGui::GetStyle().GrabRounding = 1.0f;
  ImGui::GetStyle().PopupRounding = 1.0f;
  ImGui::GetStyle().ScrollbarRounding = 1.0f;
  ImGui::GetStyle().TabRounding = 1.0f;
  ImGui::GetStyle().FrameBorderSize = 1.0f;  // original 2.f
  ImGui::GetStyle().ScrollbarSize = 18.f;

  ImVec4* colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 0.9f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.007843f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
  // colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
  colors[ImGuiCol_Border] = ImVec4(0.236f, 0.236f, 0.236f, 1.000f);  // darken
  colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
  colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
  colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
  colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
  colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  // TODO see docking branch of ImGui
  colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 0.9f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
  colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavWindowingHighlight] =
      ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
#endif
}

void deinitialize_gui_and_window(GLFWwindow* window) {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void transform_window(float* vecTranslation, float* vecRotation,
                      float* vecScale,
                      ImGuizmo::OPERATION& current_gizmo_operation,
                      ImGuizmo::MODE& current_gizmo_mode, int* mode,
                      bool* show_gizmo, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("Transform manipulator", open)) {
    if (ImGui::RadioButton("Mesh mode", *mode == 0)) *mode = 0;
    ImGui::SameLine();
    if (ImGui::RadioButton("Bone mode", *mode == 1)) *mode = 1;
    ImGui::Separator();

    ImGui::InputFloat3("Tr", vecTranslation, 3);
    ImGui::InputFloat3("Rt", vecRotation, 3);
    ImGui::InputFloat3("Sc", vecScale, 3);

    if (ImGui::Button("Reset transforms")) {
      if (*mode == 0) {
        vecTranslation[0] = 0;
        vecTranslation[1] = 0;
        vecTranslation[2] = 0;
        vecRotation[0] = 0;
        vecRotation[1] = 0;
        vecRotation[2] = 0;
        vecScale[0] = 1;
        vecScale[1] = 1;
        vecScale[2] = 1;
      } else {
        // TODO do we reset to bone current animation pose, or do we reset to
        // bone binding pose?

        // also, we don't have the data to do that in this functions as of now
      }
    }

    if (show_gizmo) {
      ImGui::SameLine();
      ImGui::Checkbox("Show Gizmo?", show_gizmo);
    }

    ImGui::Separator();

    if (ImGui::RadioButton("Translate",
                           current_gizmo_operation == ImGuizmo::TRANSLATE))
      current_gizmo_operation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate",
                           current_gizmo_operation == ImGuizmo::ROTATE))
      current_gizmo_operation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", current_gizmo_operation == ImGuizmo::SCALE))
      current_gizmo_operation = ImGuizmo::SCALE;

    ImGui::Separator();
    if (ImGui::RadioButton("World", current_gizmo_mode == ImGuizmo::WORLD))
      current_gizmo_mode = ImGuizmo::WORLD;
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", current_gizmo_mode == ImGuizmo::LOCAL))
      current_gizmo_mode = ImGuizmo::LOCAL;
  }
  ImGui::End();
}

void timeline_window(gltf_insight::AnimSequence loaded_sequence,
                     bool& playing_state, bool& need_to_update_pose,
                     bool& looping, int& selectedEntry, int& firstFrame,
                     bool& expanded, int& currentFrame, double& currentPlayTime,
                     bool* open, float docked_size_prop,
                     float docked_size_max_pixel) {
  if (open && !*open) return;
  const auto display_size = ImGui::GetIO().DisplaySize;
  const auto dockspace_pixel_size =
      std::min(docked_size_max_pixel, docked_size_prop * display_size.y);

  ImGui::SetNextWindowSize(ImVec2(display_size.x, dockspace_pixel_size));
  ImGui::SetNextWindowPos(ImVec2(0, display_size.y - dockspace_pixel_size));

  if (ImGui::Begin("Timeline", open,
                   ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoCollapse)) {
    const auto adjust_time = [&] {
      currentPlayTime = double(currentFrame) / double(ANIMATION_FPS);
      need_to_update_pose = true;
    };

    if (ImGui::Button(ICON_II_SKIP_BACKWARD)) {
      currentFrame = 0;
      adjust_time();
    }

    // play pause button
    ImGui::SameLine();
    if (ImGui::Button(playing_state ? ICON_II_PAUSE : ICON_II_PLAY))
      playing_state = !playing_state;

    // stop button
    ImGui::SameLine();
    if (ImGui::Button(ICON_II_STOP)) {
      playing_state = false;
      currentFrame = 0;
      adjust_time();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_II_SKIP_FORWARD)) {
      currentFrame = loaded_sequence.mFrameMax;
      adjust_time();
    }

    ImGui::SameLine();
    ImGui::Checkbox(ICON_II_LOOP " looping", &looping);

    ImGui::SameLine();

    ImGui::PushItemWidth(180);
    ImGui::InputInt("Frame Min ", &loaded_sequence.mFrameMin);
    ImGui::SameLine();
    if (ImGui::InputInt("Frame ", &currentFrame)) {
      adjust_time();
    }
    ImGui::SameLine();
    ImGui::InputInt("Frame Max ", &loaded_sequence.mFrameMax);
    ImGui::PopItemWidth();

    const auto saved_frame = currentFrame;
    Sequencer(&loaded_sequence, &currentFrame, &expanded, &selectedEntry,
              &firstFrame, ImSequencer::SEQUENCER_CHANGE_FRAME);
    if (saved_frame != currentFrame) {
      adjust_time();
    }

    // add a UI to edit that particular item
    if (selectedEntry != -1) {
      const gltf_insight::AnimSequence::AnimSequenceItem& item =
          loaded_sequence.myItems[size_t(selectedEntry)];
      ImGui::Text("I am a %s, please edit me",
                  gltf_insight::SequencerItemTypeNames[item.mType]);
      // switch (type) ....
    }
  }
  ImGui::End();
}

void shader_selector_window(const std::vector<std::string>& shader_names,
                            int& selected_shader, std::string& shader_to_use,
                            int& display_mode, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("Shader mode", open)) {
    ImGui::Text("Display_mode:");
    if (ImGui::RadioButton("Normal", display_mode == 0)) display_mode = 0;
    if (ImGui::RadioButton("Debug", display_mode == 42)) display_mode = 42;

    if (display_mode == 0x2A) {
      ImGuiCombo("Choose shader", &selected_shader, shader_names);
      shader_to_use = shader_names[size_t(selected_shader)];
    }
  }
  ImGui::End();
}

void animation_window(std::vector<animation>& animations, bool* open) {
  if (open && !*open) return;

  static int selected_animation_index = 0;
  static int selected_animation_channel_index;

  static std::vector<std::string> animation_names;

  // Copying a bunch of strings at each frame will slow down the program, we
  // keep a cached list of the animations names
  // We can quicly test if we need to rebuild the animation_name lists:
  bool rebuild_name_list = false;
  if (animations.size() != animation_names.size())
    rebuild_name_list = true;
  else {
    for (size_t i = 0; i < animations.size(); ++i) {
      if (animation_names[i] != animations[i].name) {
        rebuild_name_list = true;
        break;
      }
    }
  }

  // Rebuild efficiently the list of name for the combo box
  if (rebuild_name_list) {
    // allocate all the std::strings at once
    animation_names.resize(animations.size());
    for (size_t i = 0; i < animations.size(); ++i) {
      animation_names[i] = animations[i].name;
    }
  }

  // ImGui window
  if (ImGui::Begin("Animation explorer", open)) {
    // Not all glTF have animations, ignore the rest of the code if there's none
    if (animations.empty()) {
      // Display a message that expalin why windo is empty in a cool orange
      // shade
      ImGui::TextColored(
          ImVec4(1, .5, 0, 1),
          "The currently loaded glTF doesn't have any animations");
    }

    // Display the animation motion widget
    else {
      // This is likely due to loading another glTF asset
      if (selected_animation_index >= int(animations.size()))
        selected_animation_index = 0;
      ImGuiCombo("Animations", &selected_animation_index, animation_names);

      animation& selected_animation =
          animations[size_t(selected_animation_index)];

      // We are going to mix in the view the sampler data and the channel data
      // as editing them separately makes little sense.
      ImGui::Text("Current Animation [%s]", selected_animation.name.c_str());
      ImGui::Text("Contains [%zu] channels",
                  selected_animation.channels.size());
      ImGui::Separator();

      // Propose to change the channel we dsiplay why being sure we point to a
      // correct one in the array
      if (selected_animation_channel_index >=
          int(selected_animation.channels.size()))
        selected_animation_channel_index = 0;
      ImGui::InputInt("Channel to display", &selected_animation_channel_index,
                      1, 1);
      selected_animation_channel_index =
          glm::clamp<int>(selected_animation_channel_index, 0,
                          int(selected_animation.channels.size()) - 1);

      // Get the channel object :
      auto& channel =
          selected_animation.channels[size_t(selected_animation_channel_index)];

      // Get the associated sampler object :
      auto& sampler =
          selected_animation.samplers[size_t(channel.sampler_index)];

      ImGui::Text("target node [%d] path [%s]", channel.target_node, [channel] {
        switch (channel.mode) {
          case animation::channel::path::translation:
            return "translation";
          case animation::channel::path::rotation:
            return "rotation";
          case animation::channel::path::scale:
            return "scale";
          case animation::channel::path::weight:
            return "weight";
          case animation::channel::path::not_assigned:
            return "ERROR";
        }
        return "ERROR";
      }());

      bool is_cubic_spline = false;
      ImGui::Text("Animation channel has [%zu] keyframes",
                  sampler.keyframes.size());
      ImGui::Text("Sampler is set to [%s] interpolation_mode", [&] {
        switch (sampler.mode) {
          case animation::sampler::interpolation::linear:
            return "LINEAR";
          case animation::sampler::interpolation::step:
            return "STEP";
          case animation::sampler::interpolation::cubic_spline:
            is_cubic_spline = true;
            return "CUBICSPLINE";
          case animation::sampler::interpolation::not_assigned:
            return "ERROR";
        }
        return "ERROR";
      }());

      const auto column_count = [channel]() -> int {
        switch (channel.mode) {
          case animation::channel::path::scale:
          case animation::channel::path::translation:
            return 4;
          case animation::channel::path::rotation:
            return 5;
          case animation::channel::path::weight:
            return 2;
          case animation::channel::path::not_assigned:
            return -1;
        }
        return -1;
      }();

      if (column_count == -1) {
        // error
      }

      ImGui::Separator();
      ImGui::Columns(column_count);
      ImGui::TextColored(ImVec4(1, .5, 0, 1), "TimePoint");
      ImGui::NextColumn();
      if (column_count == 2) {
        ImGui::TextColored(ImVec4(1, .5, 0, 1), "Weight");
        ImGui::NextColumn();
      } else
        for (size_t i = 1; i < size_t(column_count); ++i) {
          static const char vector_comp[] = "XYZW";
          ImGui::TextColored(ImVec4(1, .5, 0, 1), "%c", vector_comp[i - 1]);
          ImGui::NextColumn();
        }

      for (size_t frame = 0; frame < sampler.keyframes.size(); ++frame) {
        const std::string keyframe_input_name =
            "###"
            "keyframe_input" +
            std::to_string(frame);

        ImGui::PushItemWidth(-1);
        ImGui::InputFloat(keyframe_input_name.c_str(),
                          &sampler.keyframes[frame].second);
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        for (size_t c = 1; c < size_t(column_count); ++c) {
          size_t i = c - 1;

          if (is_cubic_spline) {
            for (size_t cs_c = 0; cs_c < 3; cs_c++) {
              const auto cs_frame = 3 * frame + cs_c;
              if (i == 0) {
                switch (cs_c) {
                  case 0:
                    ImGui::Text("InTan ");
                    break;
                  case 1:
                    ImGui::Text("KeyVal");
                    break;
                  case 2:
                    ImGui::Text("OutTan");
                    break;
                  default:
                    break;
                }
                ImGui::SameLine();
              }
              const std::string keyframe_frame_comp_name =
                  "###"
                  "keyframe_input" +
                  std::to_string(frame) + "comp" + std::to_string(c) +
                  "cubic_spline" + std::to_string(cs_c);

              float* value_to_manipulate = [&]() -> float* {
                switch (channel.mode) {
                  case animation::channel::path::translation:
                    return &channel.keyframes[size_t(cs_frame)]
                                .second.motion.translation[int(i)];
                  case animation::channel::path::rotation:
                    return &channel.keyframes[size_t(cs_frame)]
                                .second.motion.rotation[int(i)];
                  case animation::channel::path::scale:
                    return &channel.keyframes[size_t(cs_frame)]
                                .second.motion.scale[int(i)];
                  case animation::channel::path::weight:
                    return &channel.keyframes[size_t(cs_frame)]
                                .second.motion.weight;
                  case animation::channel::path::not_assigned:
                    return nullptr;
                }
                return nullptr;
              }();

              if (value_to_manipulate) {
                ImGui::PushItemWidth(-1);
                ImGui::InputFloat(keyframe_frame_comp_name.c_str(),
                                  value_to_manipulate, 0, 0, "%.6f");
                ImGui::PopItemWidth();
              }
            }
          } else {
            const std::string keyframe_frame_comp_name =
                "###"
                "keyframe_input" +
                std::to_string(frame) + "comp" + std::to_string(c);
            float* value_to_manipulate = [&]() -> float* {
              switch (channel.mode) {
                case animation::channel::path::translation:
                  return &channel.keyframes[size_t(frame)]
                              .second.motion.translation[int(i)];
                case animation::channel::path::rotation:
                  return &channel.keyframes[size_t(frame)]
                              .second.motion.rotation[int(i)];
                case animation::channel::path::scale:
                  return &channel.keyframes[size_t(frame)]
                              .second.motion.scale[int(i)];
                case animation::channel::path::weight:
                  return &channel.keyframes[size_t(frame)].second.motion.weight;
                case animation::channel::path::not_assigned:
                  return nullptr;
              }
              return nullptr;
            }();

            if (value_to_manipulate) {
              ImGui::PushItemWidth(-1);
              ImGui::InputFloat(keyframe_frame_comp_name.c_str(),
                                value_to_manipulate, 0, 0, "%.6f");
              ImGui::PopItemWidth();
            }
          }

          ImGui::NextColumn();
        }
        ImGui::Separator();
      }
    }
  }
  ImGui::End();
}

void camera_parameters_window(float& fovy, float& z_far, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("Camera Parameters", open)) {
    ImGui::SliderFloat("FOV", &fovy, 5, 90, "%.1f");
    ImGui::SliderFloat("draw distance", &z_far, 100, 1000, "%.0f");
  }
  ImGui::End();
}
#include "IconsIonicons.h"
#include "cmake_config.hh"
void about_window(GLuint logo, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("About glTF-insight", open, ImVec2(1024, 315))) {
    ImGui::SetWindowFontScale(1.25);
    ImGui::Columns(2);
    ImGui::SetColumnWidth(-1, 256 + ImGui::GetStyle().WindowPadding.x);
    ImGui::Image(ImTextureID(size_t(logo)), ImVec2(256, 256));
    ImGui::NextColumn();
    ImGui::Text("glTF-insight\n\n");
#if CMAKE_CI
#define LOCAL_ID "%d"
#else
#define LOCAL_ID "%x"
#endif

#ifdef __clang__
#pragma clang diagnostic push
#if __has_warning("-Wunreachable-code")
#pragma clang diagnostic ignored "-Wunreachable-code"
#endif
#endif

    ImGui::Text("Version %d.%d.%d." LOCAL_ID ". Platform %s", CMAKE_BUILD_MAJOR,
                CMAKE_BUILD_MINOR, CMAKE_BUILD_PATCH,
                CMAKE_CI ? CMAKE_CI_BUILD : 0x1337F00D,
                os_utils::platform().c_str());
#undef LOCAL_ID

    ImGui::Text("Built from git commit " CMAKE_GIT_COMMIT_SHORT);

    if (CMAKE_CI) ImGui::Text("Continuous Built from " CMAKE_CI_NAME);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    ImGui::Text("\n");
    ImGui::Text(
        u8"\u00A9"  // (c) symbol unicode +00A9
        " "
        "2018-2019 Light Transport Entertainment, Inc.; "
        "Syoyo Fujita; "
        "Arthur Brainville; "
        "and contributors."
        "\n");

    ImGui::Text(
        "\t"
        "Licensed under the terms of the MIT license agreement"
        "\n\n");

    ImGui::Text(
        "This program is Free (Libre) and Open-Source. It's "
        "development is "
        "hosted on GitHub");

    ImGui::Text("\t" ICON_II_SOCIAL_GITHUB);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0, 1, 1, 1),
                       " "
                       "https://github.com/ybalrd/gltf-insight"
                       "\n\n");

    ImGui::Text(
        "glTF and the glTF logo are trademarks of the Khronos "
        "Group "
        "Inc.");
    ImGui::NextColumn();
    ImGui::Columns();
    ImGui::SetWindowFontScale(1);
  }
  ImGui::End();
}

void material_info_window(gltf_insight::material& dummy,
                          std::vector<gltf_insight::material>& loaded_materials,
                          bool* open) {
  static constexpr int tsize = 128;
  if (open && !*open) return;
  if (ImGui::Begin("Materials", open)) {
    static int index = -1;
    ImGui::Text("The currently asset contains %d materials",
                int(loaded_materials.size()));

    ImGui::InputInt("Material index", &index);
    index = glm::clamp(index, -1, int(loaded_materials.size()) - 1);

    auto& selected = index != -1 && (index < int(loaded_materials.size()))
                         ? loaded_materials[size_t(index)]
                         : dummy;

    ImGui::Text("Name: %s", selected.name.c_str());
    ImGui::Text("Type: %s",
                gltf_insight::to_string(selected.intended_shader).c_str());

    ImGui::ColorEdit3(
        "Emissive factor", glm::value_ptr(selected.emissive_factor),
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
    ImGui::Text("Alpha Mode [%s]", to_string(selected.alpha_mode).c_str());
    ImGui::Text("Alpha cutoff [%.3f]", double(selected.alpha_cutoff));
    auto read_only_bool = selected.double_sided;
    ImGui::Checkbox("Double Sided", &read_only_bool);

    ImGui::Separator();
    ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_DockingPreview],
                       "Texture maps:");
    ImGui::Columns(3);
    ImGui::Text(ICON_II_ARROW_RIGHT_B " Normal");
    ImGui::NextColumn();
    ImGui::Text(ICON_II_ARROW_RIGHT_B " Occlusion");
    ImGui::NextColumn();
    ImGui::Text(ICON_II_ARROW_RIGHT_B " Emissive");
    ImGui::NextColumn();
    ImGui::Image(ImTextureID(size_t(selected.normal_texture)),
                 ImVec2(tsize, tsize));
    ImGui::NextColumn();
    ImGui::Image(ImTextureID(size_t(selected.occlusion_texture)),
                 ImVec2(tsize, tsize));
    ImGui::NextColumn();
    ImGui::Image(ImTextureID(size_t(selected.emissive_texture)),
                 ImVec2(tsize, tsize));
    ImGui::NextColumn();
    ImGui::Columns();
    ImGui::Separator();
    ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_DockingPreview],
                       "Shader specific data:");
    switch (selected.intended_shader) {
      case gltf_insight::shading_type::pbr_metal_rough: {
        auto& pbr_metal_rough = selected.shader_inputs.pbr_metal_roughness;
        ImGui::Columns(2);
        ImGui::ColorEdit4(
            "Base Color Factor",
            glm::value_ptr(pbr_metal_rough.base_color_factor),
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
        ImGui::NextColumn();
        ImGui::Text("Roughness [%.3f]\nMetallic_factor [%.3f]",
                    double(pbr_metal_rough.roughness_factor),
                    double(pbr_metal_rough.metallic_factor));
        ImGui::NextColumn();
        ImGui::Text(ICON_II_ARROW_RIGHT_B " Albedo (BaseColor) Map:");
        ImGui::NextColumn();
        ImGui::Text(ICON_II_ARROW_RIGHT_B " Metal Roughness map:");
        ImGui::NextColumn();
        ImGui::Image(ImTextureID(size_t(pbr_metal_rough.base_color_texture)),
                     ImVec2(tsize, tsize));

        ImGui::NextColumn();
        ImGui::Image(
            ImTextureID(size_t(pbr_metal_rough.metallic_roughness_texture)),
            ImVec2(tsize, tsize));
        ImGui::NextColumn();
        ImGui::Columns();
      } break;

      case gltf_insight::shading_type::pbr_specular_glossy: {
      } break;

      case gltf_insight::shading_type::unlit: {
        auto& unlit = selected.shader_inputs.unlit;
        ImGui::ColorEdit4(
            "Base Color Factor", glm::value_ptr(unlit.base_color_factor),
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
        ImGui::Image(ImTextureID(size_t(unlit.base_color_texture)),
                     ImVec2(2 * tsize, 2 * tsize));

      } break;
    }
  }
  ImGui::End();
}

static void scene_outline_window_recur(gltf_node& node) {
  std::string node_name;

  if (node.type == gltf_node::node_type::mesh) node_name += "mesh ";
  if (node.type == gltf_node::node_type::bone) node_name += "joint ";

  node_name += "node " + std::to_string(node.gltf_node_index);
  if (ImGui::TreeNode(node_name.c_str())) {
    if (node.type == gltf_node::node_type::mesh)
      ImGui::Text("Mesh %d", node.gltf_mesh_id);
    ImGui::Text("local_xform");
    ImGui::Columns(4);
    for (int line = 0; line < 4; line++)
      for (int col = 0; col < 4; col++) {
        // ImGui::InputFloat("###",
        // (float*)&glm::value_ptr(node.local_xform)[i]);
        ImGui::Text("%.3f", double(node.local_xform[col][line]));
        ImGui::NextColumn();
      }
    ImGui::Columns();

    for (auto child : node.children) {
      scene_outline_window_recur(*child);
    }

    ImGui::TreePop();
  }
}
void scene_outline_window(gltf_node& scene, bool* open) {
  if (open && !*open) return;
  if (ImGui::Begin("Scene outline", open)) scene_outline_window_recur(scene);
  ImGui::End();
}

void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods) {
  (void)mods;

  auto* param =
      &(reinterpret_cast<gltf_insight::app*>(glfwGetWindowUserPointer(window))
            ->gui_parameters);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    param->button_states[0] = true;
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    param->button_states[1] = true;
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    param->button_states[2] = true;
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_FALSE)
    param->button_states[0] = false;
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_FALSE)
    param->button_states[1] = false;
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_FALSE)
    param->button_states[2] = false;
}

void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y) {
  auto* param =
      &(reinterpret_cast<gltf_insight::app*>(glfwGetWindowUserPointer(window))
            ->gui_parameters);

  // mouse left pressed
  if (param->button_states[0] && !ImGui::GetIO().WantCaptureMouse &&
      !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
    param->rot_yaw -= param->rotation_scale * (mouse_x - param->last_mouse_x);
    param->rot_pitch -= param->rotation_scale * (mouse_y - param->last_mouse_y);
    param->rot_pitch = glm::clamp(param->rot_pitch, -90.0, +90.0);
  }

  param->last_mouse_x = mouse_x;
  param->last_mouse_y = mouse_y;
}

GLuint load_gltf_insight_icon() {
  int w, h, c;
  stbi_uc* data = nullptr;
  GLuint gl_image;
  data = stbi_load_from_memory(gltf_insight_256_png,
                               int(gltf_insight_256_png_len), &w, &h, &c, 4);

  if (!data) {
    throw std::runtime_error("Could not load gltf-insight logo!");
  }

  glGenTextures(1, &gl_image);
  glBindTexture(GL_TEXTURE_2D, gl_image);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               data);

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(data);
  return gl_image;
}
