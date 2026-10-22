#include "../external/glad/include/glad/glad.h"
#include "../external/stb_image.h"
#include <GLFW/glfw3.h>

#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>

struct ShaderSource
{
    std::string vertex_source;
    std::string fragment_source;
};

struct rimg
{
    GLFWwindow*                         window;
    std::vector<float>                  vertices;
    std::vector<unsigned int>           indices;
    unsigned char*                      img_data;
    int                                 window_width;
    int                                 window_height;
    int                                 img_width;
    int                                 img_height;
    unsigned int                        vertex_array;
    unsigned int                        vertex_buffer;
    unsigned int                        index_buffer;
    unsigned int                        vertex_shader;
    unsigned int                        fragment_shader;
    unsigned int                        shader_program;
    unsigned int                        texture;
};

auto process_input(GLFWwindow* window) -> void
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

auto framebuffer_size_callback(GLFWwindow* window, int width, int height) -> void
{
    glViewport(0, 0, width, height);
}

auto load_glad() -> bool {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!\n";
        return false;
    }
    return true;
}

auto load_image(std::string_view img_path, rimg* img) -> bool
{
    int w, h, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* img_data {stbi_load(img_path.data(), &w, &h, &channels, 4)};
    if (!img_data) return false;

    img->img_data = img_data;
    img->img_width = w;
    img->img_height = h;
    return true;
}

auto compile_shader(const ShaderSource& source, rimg* img) -> bool
{
    unsigned int vid {glCreateShader(GL_VERTEX_SHADER)};
    const char* vsrc {source.vertex_source.c_str()};
    glShaderSource(vid, 1, &vsrc, nullptr);
    glCompileShader(vid);

    int vertex_compile_result;
    glGetShaderiv(vid, GL_COMPILE_STATUS, &vertex_compile_result);
    if (vertex_compile_result == GL_FALSE)
    {
        int len;
        glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log_msg(len);
        glGetShaderInfoLog(vid, len, &len, log_msg.data());
        std::cerr << "Failed to compile vertex shader!\n";
        for (const auto& c : log_msg) std::cerr << c;
        std::cout << '\n';
        return false;
    }

    unsigned int fid {glCreateShader(GL_FRAGMENT_SHADER)};
    const char* fsrc {source.fragment_source.c_str()};
    glShaderSource(fid, 1, &fsrc, nullptr);
    glCompileShader(fid);

    int fragment_compile_result;
    glGetShaderiv(fid, GL_COMPILE_STATUS, &fragment_compile_result);
    if (fragment_compile_result == GL_FALSE)
    {
        int len;
        glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log_msg(len);
        glGetShaderInfoLog(fid, len, &len, log_msg.data());
        std::cerr << "Failed to compile fragment shader!\n";
        return false;
    }

    img->vertex_shader = vid;
    img->fragment_shader = fid;
    return true;
}

auto generate_shader_program(rimg* img) -> bool
{
    img->shader_program = glCreateProgram();
    glAttachShader(img->shader_program, img->vertex_shader);
    glAttachShader(img->shader_program, img->fragment_shader);
    glLinkProgram(img->shader_program);

    int shader_program_link_result;
    glGetProgramiv(img->shader_program, GL_LINK_STATUS,&shader_program_link_result);
    if (shader_program_link_result == GL_FALSE)
    {
        int len;
        glGetProgramiv(img->shader_program, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log_msg(len);
        glGetProgramInfoLog(img->shader_program, len, &len, log_msg.data());
        std::cerr << "Failed to link shader program.\n";
        return false;
    }
    return true;
}

auto parse_shader(const char* img_path) -> ShaderSource
{
    enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };

    std::ifstream shader_file {img_path};
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (std::getline(shader_file, line))
    {
        if (line.find("shader") != std::string::npos)
        {
            if (line.contains("vertex")) type = ShaderType::VERTEX;
            if (line.contains("fragment")) type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[static_cast<int>(type)] << line << '\n';
        }
    }

    return {ss[0].str(), ss[1].str()};
}

auto clean_up(rimg* img) -> void
{
    if (img->img_data) stbi_image_free(img->img_data);
    glDeleteProgram(img->shader_program);
    glDeleteVertexArrays(1, &img->vertex_array);
    glDeleteBuffers(1, &img->vertex_buffer);
    glDeleteBuffers(1, &img->index_buffer);
    glDeleteTextures(1, &img->texture);
    glfwTerminate();
}

auto main(int argc, char** argv) -> int
{
    if (argc == 1)
    {
        std::cerr << "usage: rimg [path/to/image]\n";
        return 1;
    }

    rimg img {};
    img.window_width = 800;
    img.window_height = 600;
    assert(load_image(argv[1], &img));

    assert(glfwInit());
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    img.window = glfwCreateWindow(img.window_width, img.window_height, "rimg", nullptr, nullptr);
    if (!img.window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(img.window);
    if (!load_glad()) return 1;
    glfwSetFramebufferSizeCallback(img.window, framebuffer_size_callback);

    std::vector<float> vertices {
        // positions            // texture coordinates
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f,
    };

    std::vector<unsigned int> indices {
        0, 1, 2,
        0, 3, 2
    };

    img.vertices = vertices;
    img.indices = indices;

    ShaderSource source {parse_shader("src/rimg.shader")};
    assert(compile_shader(source, &img));
    generate_shader_program(&img);
    glUseProgram(img.shader_program);

    glGenVertexArrays(1, &img.vertex_array);
    glBindVertexArray(img.vertex_array);

    glGenBuffers(1, &img.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, img.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * img.vertices.size(), img.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &img.index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, img.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * img.indices.size(), img.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    auto tex_offset {3 * sizeof(float)};
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) tex_offset);
    glEnableVertexAttribArray(1);

    glGenTextures(1, &img.texture);
    glBindTexture(GL_TEXTURE_2D, img.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.img_width, img.img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.img_data);
    // glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    while (!glfwWindowShouldClose(img.window))
    {
        glUseProgram(img.shader_program);
        process_input(img.window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, img.texture);
        glBindVertexArray(img.vertex_array);
        glDrawElements(GL_TRIANGLES, img.indices.size(), GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(img.window);
        glfwPollEvents();
    }

    clean_up(&img);
    return 0;
}
