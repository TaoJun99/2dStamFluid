#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

// Grid dimensions
const int GRID_SIZE = 128;

// Fullscreen Quad Vertices
float quadVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f,  1.0f,
        -1.0f,  1.0f
};

unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
};

// Global variables
GLuint addDyeShaderProgram;
GLuint advectShaderProgram;
GLuint jacobiShaderProgram;
GLuint dyeTexture;
GLuint velocityTexture;
GLuint pressureTexture;
GLuint framebuffer;
GLuint VAO, VBO, EBO;

GLuint compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Error:\n" << infoLog << std::endl;
    }
    return shader;
}

// Function to read shader source from file
std::string readShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readShaderSource(vertexPath);
    std::string fragmentSource = readShaderSource(fragmentPath);

    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program Linking Error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

void addDye(GLFWwindow *window, bool click) {
    // Get normalized mouse coordinates
    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Get window size
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

// Normalize mouse coordinates
//    float u = static_cast<float>(mouseX) / static_cast<float>(windowWidth);
    float u = (float) mouseX / windowWidth;
    float v = (float) 1.0f - mouseY / windowHeight;
//    float v = 1.0f - static_cast<float>(mouseY) / static_cast<float>(windowHeight); // Invert Y-axis


    glUseProgram(addDyeShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dyeTexture, 0);

    GLuint mousePosLoc = glGetUniformLocation(addDyeShaderProgram, "mousePos");
    GLuint dyeRadiusLoc = glGetUniformLocation(addDyeShaderProgram, "dyeRadius");
    GLuint dyeColorLoc = glGetUniformLocation(addDyeShaderProgram, "dyeColor");
    GLuint addDyeLoc = glGetUniformLocation(addDyeShaderProgram, "addDye");

    glUniform2f(mousePosLoc, u, v);
    glUniform1f(dyeRadiusLoc, 0.1);
    GLfloat dyeColor[3] = { 0.2f, 0.0f, 0.0f };
    glUniform3fv(dyeColorLoc, 1, dyeColor);
    glUniform1i(addDyeLoc, click);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void advect(bool isAdvectDye) {
    glUseProgram(advectShaderProgram);

    // Bind the velocity and dye textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);

    // Set the uniform variables
    GLuint timestepLoc = glGetUniformLocation(advectShaderProgram, "timestep");
    GLuint rdxLoc = glGetUniformLocation(advectShaderProgram, "rdx");
    GLuint dyeTextureLoc = glGetUniformLocation(advectShaderProgram, "advectedTexture");
    GLuint velocityTextureLoc = glGetUniformLocation(advectShaderProgram, "velocityTexture");
    GLuint isAdvectDyeLoc = glGetUniformLocation(advectShaderProgram, "isAdvectDye");

    glUniform1f(timestepLoc, 0.1);
    glUniform1f(rdxLoc, 1.0f / GRID_SIZE);
    glUniform1i(dyeTextureLoc, 0);
    glUniform1i(velocityTextureLoc, 1);
    if (isAdvectDye) {
        glUniform1i(isAdvectDyeLoc, 1);
    } else {
        glUniform1i(isAdvectDyeLoc, 0);
    }


    // Render texture to framebuffer
    GLuint tempframebuffer;
    glGenFramebuffers(1, &tempframebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, tempframebuffer);
    if (isAdvectDye) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dyeTexture, 0);
    } else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexture, 0);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &tempframebuffer);
}

void diffuse() {
    glUseProgram(jacobiShaderProgram);
    // Bind the velocity texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);

    // Uniform variables
    GLuint alphaLoc = glGetUniformLocation(jacobiShaderProgram, "alpha");
    GLuint rBetaLoc = glGetUniformLocation(jacobiShaderProgram, "rBeta");
    GLuint xLoc = glGetUniformLocation(jacobiShaderProgram, "x");
    GLuint bLoc = glGetUniformLocation(jacobiShaderProgram, "b");

    float dx = 1.0 / GRID_SIZE;
    float nu = 1.0;
    float timestep = 0.1;
    // dx = 1.0 / GRID_SIZE;
    float alpha = 1.0 / (nu * timestep * GRID_SIZE * GRID_SIZE);
    glUniform1f(alphaLoc, alpha);
    glUniform1f(rBetaLoc, 1.0 / ((4 + alpha)));
    glUniform1i(xLoc, 1);
    glUniform1i(bLoc, 1);

//    GLuint outputTexture;

    // Render texture to framebuffer
    GLuint tempframebuffer;
    glGenFramebuffers(1, &tempframebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, tempframebuffer);

    for (int i = 0; i < 50; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexture, 0);


        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &tempframebuffer);
}


int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL version and core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required for macOS

    GLFWwindow* window = glfwCreateWindow(800, 800, "Fluid Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    // Compile shaders and create program
    GLuint shaderProgram = createShaderProgram("../shader.vert", "../shader.frag");
    addDyeShaderProgram = createShaderProgram("../shader.vert", "../addDyeShader.frag");
    advectShaderProgram = createShaderProgram("../shader.vert", "../advect.frag");
    jacobiShaderProgram = createShaderProgram("../shader.vert", "../jacobi.frag");

    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create textures
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &dyeTexture);
    glBindTexture(GL_TEXTURE_2D, dyeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &velocityTexture);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &pressureTexture);
    glBindTexture(GL_TEXTURE_2D, pressureTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // Create framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dyeTexture, 0);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    } else {
        std::cout << "Framebuffer complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
//            std::cout << "Mouse clicked!" << std::endl;
            addDye(window, true);
        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
            addDye(window, false);
        }

        advect(true);
        advect(false);



        // First pass: render to framebuffer
//        glViewport(0, 0, GRID_SIZE, GRID_SIZE);
//        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        glUseProgram(whiteShaderProgram);
//
//        glBindVertexArray(VAO);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//        glBindVertexArray(0);


        // --- Debug: Read pixels from the framebuffer (starting from the bottom-left corner) ///
//        unsigned char* pixels = new unsigned char[GRID_SIZE * GRID_SIZE * 3];  // 3 bytes for RGB

//        glReadPixels(0, 0, GRID_SIZE, GRID_SIZE, GL_RGB, GL_UNSIGNED_BYTE, pixels);
//
//        std::cout << "First pixel (RGB): "
//                  << (int)pixels[0] << ", "
//                  << (int)pixels[1] << ", "
//                  << (int)pixels[2] << std::endl;

//        delete[] pixels;
        // ---//

        // Second pass: render to screen
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        GLint inputTextureLocation = glGetUniformLocation(shaderProgram, "inputTexture");
        glUniform1i(inputTextureLocation, 0);  // Texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dyeTexture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap buffers to display the rendered image
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &dyeTexture);
    glDeleteFramebuffers(1, &framebuffer);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

