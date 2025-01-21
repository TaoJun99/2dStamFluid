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
GLuint applyForceShaderProgram;
GLuint divergenceShaderProgram;
GLuint gradientSubtractShaderProgram;
GLuint dyeTexture;
GLuint velocityTexture;
GLuint pressureTexture;
GLuint jacobiTexture1;
GLuint jacobiTexture2;
GLuint framebuffer;
GLuint VAO, VBO, EBO;
float timeStep = 0.1;

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
    float u = (float) mouseX / windowWidth;
    float v = (float) 1.0f - mouseY / windowHeight;

    glUseProgram(addDyeShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTexture);


    GLuint dyeTextureLoc = glGetUniformLocation(addDyeShaderProgram, "dyeTexture");
    GLuint mousePosLoc = glGetUniformLocation(addDyeShaderProgram, "mousePos");
    GLuint dyeRadiusLoc = glGetUniformLocation(addDyeShaderProgram, "dyeRadius");
    GLuint dyeColorLoc = glGetUniformLocation(addDyeShaderProgram, "dyeColor");
    GLuint addDyeLoc = glGetUniformLocation(addDyeShaderProgram, "addDye");


    glUniform1i(dyeTextureLoc, 0);
    glUniform2f(mousePosLoc, u, v);
    glUniform1f(dyeRadiusLoc, 0.1);
    GLfloat dyeColor[3] = { 0.1f, 0.0f, 0.0f };
    glUniform3fv(dyeColorLoc, 1, dyeColor);
    glUniform1i(addDyeLoc, click);


    glViewport(0, 0, GRID_SIZE, GRID_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dyeTexture, 0);
//    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void advect(bool isAdvectDye) {
    glUseProgram(advectShaderProgram);

    // Generate texture to store intermediate results
    GLfloat zeroData[GRID_SIZE * GRID_SIZE * 4] = {0.0f};
    GLuint outputTexture;
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Bind the velocity and dye textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, outputTexture);


    // Set the uniform variables
    GLuint timestepLoc = glGetUniformLocation(advectShaderProgram, "timestep");
    GLuint rdxLoc = glGetUniformLocation(advectShaderProgram, "rdx");
    GLuint dyeTextureLoc = glGetUniformLocation(advectShaderProgram, "dyeTexture");
    GLuint velocityTextureLoc = glGetUniformLocation(advectShaderProgram, "velocityTexture");
    GLuint isAdvectDyeLoc = glGetUniformLocation(advectShaderProgram, "isAdvectDye");

    glUniform1f(timestepLoc, timeStep);
    glUniform1f(rdxLoc, 1.0 / GRID_SIZE);
    glUniform1i(dyeTextureLoc, 0);
    glUniform1i(velocityTextureLoc, 1);
    if (isAdvectDye) {
        glUniform1i(isAdvectDyeLoc, 1);
    } else {
        glUniform1i(isAdvectDyeLoc, 0);
    }


    // Render texture to framebuffer
//    GLuint tempframebuffer;
//    glGenFramebuffers(1, &framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

//    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    if (isAdvectDye) {
        glBindTexture(GL_TEXTURE_2D, dyeTexture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GRID_SIZE, GRID_SIZE);
    } else {
        glBindTexture(GL_TEXTURE_2D, velocityTexture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GRID_SIZE, GRID_SIZE);
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

//    glDeleteFramebuffers(1, &tempframebuffer);
    glDeleteTextures(1, &outputTexture);
}

void jacobi(GLuint outputTexture, GLuint xLoc) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 1st iteration
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, jacobiTexture1, 0);

    if (outputTexture == velocityTexture) {
        glUniform1i(xLoc, 1);
    } else if (outputTexture == pressureTexture) {
        glUniform1i(xLoc, 2);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    int NO_OF_ITERATIONS = 50;
    for (int i = 0; i < NO_OF_ITERATIONS; i++) {
        // Alternate between two textures to read & write
        if (i % 2 == 0) { // Multiple of 2 - input: jacobiTexture1, output: jacobiTexture2
            // Bind output texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, jacobiTexture2, 0);
            // Input texture
            glUniform1i(xLoc, 3);
        } else {// input: jacobiTexture2, output: jacobiTexture1
            // Bind output texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, jacobiTexture1, 0);
            // Input texture
            glUniform1i(xLoc, 4);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    // Copy final texture from framebuffer to outputTexture
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GRID_SIZE, GRID_SIZE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void diffuse() {
    glUseProgram(jacobiShaderProgram);

    // Uniform variables
    GLuint alphaLoc = glGetUniformLocation(jacobiShaderProgram, "alpha");
    GLuint rBetaLoc = glGetUniformLocation(jacobiShaderProgram, "rBeta");
    GLuint xLoc = glGetUniformLocation(jacobiShaderProgram, "x");
    GLuint bLoc = glGetUniformLocation(jacobiShaderProgram, "b");

    float dx = 1.0 / GRID_SIZE;
    float nu = 0.001;
    float alpha = (dx * dx) / (nu * timeStep);

    glUniform1f(alphaLoc, alpha);
    glUniform1f(rBetaLoc, 1.0f / (4.0f + alpha));
    glUniform1i(bLoc, 1);

    jacobi(velocityTexture, xLoc);

}



void applyForce(GLFWwindow *window) {
    // Normalized mouse coordinates
    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    float u = (float) mouseX / windowWidth;
    float v = (float) 1.0f - mouseY / windowHeight;

    glUseProgram(applyForceShaderProgram);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);

    GLuint mousePosLoc = glGetUniformLocation(applyForceShaderProgram, "mousePos");
    GLuint forceDirLoc = glGetUniformLocation(applyForceShaderProgram, "forceDir");
    GLuint forceRadiusLoc = glGetUniformLocation(applyForceShaderProgram, "forceRadius");
    GLuint forceStrengthLoc = glGetUniformLocation(applyForceShaderProgram, "forceStrength");
    GLuint velocityTextureLoc = glGetUniformLocation(applyForceShaderProgram, "velocityTexture");

    glUniform2f(mousePosLoc, u, v);
    glUniform2f(forceDirLoc, 1.0f, 1.0f);
    glUniform1f(forceRadiusLoc, 0.1f);
    glUniform1f(forceStrengthLoc, 3.0f);
    glUniform1i(velocityTextureLoc, 1);


    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexture, 0);

//    glViewport(0, 0, GRID_SIZE, GRID_SIZE);
//    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void divergence(GLuint divergenceTexture) {
    glUseProgram(divergenceShaderProgram);

//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, velocityTexture);

    GLuint wLoc = glGetUniformLocation(divergenceShaderProgram, "w");
    GLuint halfrdxLoc = glGetUniformLocation(divergenceShaderProgram, "halfrdx");

    glUniform1i(wLoc, 1);
    glUniform1f(halfrdxLoc, 1.0 / (2.0  * GRID_SIZE));

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, divergenceTexture, 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void subtractGradient() {
    GLuint outputTexture;
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glUseProgram(gradientSubtractShaderProgram);

    GLuint pLoc = glGetUniformLocation(gradientSubtractShaderProgram, "p");
    GLuint wLoc = glGetUniformLocation(gradientSubtractShaderProgram, "w");
    GLuint halfrdxLoc = glGetUniformLocation(gradientSubtractShaderProgram, "halfrdx");

    glUniform1i(pLoc, 2);
    glUniform1i(wLoc, 1);
    glUniform1f(halfrdxLoc, 1.0 / (2.0  * GRID_SIZE));

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, velocityTexture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GRID_SIZE, GRID_SIZE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void project() {
    // Divergence of intermediate velocity field w
    GLuint divergenceTexture;
    glGenTextures(1, &divergenceTexture);
    glBindTexture(GL_TEXTURE_2D, divergenceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, divergenceTexture);

    divergence(divergenceTexture);

    glUseProgram(jacobiShaderProgram);

    GLuint alphaLoc = glGetUniformLocation(jacobiShaderProgram, "alpha");
    GLuint rBetaLoc = glGetUniformLocation(jacobiShaderProgram, "rBeta");
    GLuint xLoc = glGetUniformLocation(jacobiShaderProgram, "x");
    GLuint bLoc = glGetUniformLocation(jacobiShaderProgram, "b");

    float dx = GRID_SIZE;
    float alpha = -(dx * dx);
    float rBeta = 1.0 /4.0;


    glUniform1f(alphaLoc, alpha);
    glUniform1f(rBetaLoc, rBeta);
    glUniform1i(bLoc, 5); // divergence of w

    // Solve for pressure field
    jacobi(pressureTexture, xLoc);

    subtractGradient();

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
    applyForceShaderProgram = createShaderProgram("../shader.vert", "../applyForce.frag");
    divergenceShaderProgram = createShaderProgram("../shader.vert", "../divergence.frag");
    gradientSubtractShaderProgram = createShaderProgram("../shader.vert", "../subtractGradient.frag");


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
    GLfloat zeroData[GRID_SIZE * GRID_SIZE * 4] = {0.0f};
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &dyeTexture);
    glBindTexture(GL_TEXTURE_2D, dyeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &velocityTexture);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &pressureTexture);
    glBindTexture(GL_TEXTURE_2D, pressureTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &jacobiTexture1);
    glBindTexture(GL_TEXTURE_2D, jacobiTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &jacobiTexture2);
    glBindTexture(GL_TEXTURE_2D, jacobiTexture2);
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

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            applyForce(window);
        }

        advect(false);
        advect(true);


        diffuse();

        project();



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
        glUniform1i(inputTextureLocation, 0);  // Texture unit 0: dyeTexture
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, dyeTexture);

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

