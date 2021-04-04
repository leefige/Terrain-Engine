#include "terrain_engine.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2.h>

namespace cg
{

const glm::mat4 TerrainEngine::worldModel = glm::translate(
    glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 30.0f, 50.0f)),
    glm::vec3(0.0f, 1.0f, 0.0f)
);

const GLfloat TerrainEngine::cubeVertices[] = {
    // x     y     z    texture coordinates
    // back
    -1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
    1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, 0.0f,

    // right
    1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
    1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
    1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f,  1.0f, 1.0f,

    // front
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
    1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
    1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
    1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f,

    // left
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f,

    // top
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,
    1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
    1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,

    // bottom: water
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
    1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
    1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
    1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
};

TerrainEngine::TerrainEngine() :
    heightmap_(nullptr), mapHeight_(0), mapWidth_(0), mapChannels_(0),
    waterTexture_(0), landTexture_(0), detailTexture_(0), skyboxTextures_{0},
    skyboxShader_(nullptr), waveSpeed_(0.3), waveScale_(0.3)
{
    // Set up vertex data (and buffer(s)) and attribute pointers
    glGenVertexArrays(1, &skyboxVAO_);
    glBindVertexArray(skyboxVAO_);

    glGenBuffers(1, &skyboxVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubeAttrNum * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, cubeAttrNum * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TerrainEngine::~TerrainEngine()
{
    if (heightmap_ != nullptr) {
        SOIL_free_image_data(heightmap_);
    }

    glDeleteTextures(1, &waterTexture_);
    glDeleteTextures(1, &landTexture_);
    glDeleteTextures(1, &detailTexture_);
    glDeleteTextures(5, skyboxTextures_);

    glDeleteVertexArrays(1, &skyboxVAO_);
    glDeleteBuffers(1, &skyboxVBO_);
}

bool TerrainEngine::LoadHeightmap(const char* heightmapFile)
{
    this->heightmap_ = SOIL_load_image(
        heightmapFile,
        &this->mapWidth_, &this->mapHeight_, &this->mapChannels_,
        SOIL_LOAD_L
    );
    return this->heightmap_ != nullptr;
}

bool TerrainEngine::LoadSkybox(const char* const skyboxFiles[5])
{
    for (int i = 0; i < 5; i++) {
        if ((this->skyboxTextures_[i] = LoadTexture(skyboxFiles[i])) == 0) {
            return false;
        }
    }
    return true;
}

bool TerrainEngine::LoadWaterTexture(const char* waterFile)
{
    return (this->waterTexture_ = LoadTexture(waterFile)) != 0;
}

bool TerrainEngine::LoadLandTexture(const char* landFile)
{
    return (this->landTexture_ = LoadTexture(landFile)) != 0;
}

bool TerrainEngine::LoadDetailTexture(const char* detailFile)
{
    return (this->detailTexture_ = LoadTexture(detailFile)) != 0;
}

bool TerrainEngine::InstallSkyboxShaders(const char* vert, const char* frag)
{
    this->skyboxShader_ = Shader::Create(vert, frag);
    return this->skyboxShader_ != nullptr;
}

bool TerrainEngine::InstallWaterShaders(const char* vert, const char* frag)
{
    this->waterShader_ = Shader::Create(vert, frag);
    return this->waterShader_ != nullptr;
}


void TerrainEngine::DrawSkybox(const glm::mat4& view, const glm::mat4& projection) const
{
    skyboxShader_->Use();
    glBindVertexArray(skyboxVAO_);

    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(skyboxShader_->Program(), "model");
    GLint viewLoc = glGetUniformLocation(skyboxShader_->Program(), "view");
    GLint projLoc = glGetUniformLocation(skyboxShader_->Program(), "projection");

    // Pass the matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(worldModel));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    for (int i = 0; i < 5; i++) {
        glBindTexture(GL_TEXTURE_2D, skyboxTextures_[i]);
        // erase border
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
}

void TerrainEngine::DrawWater(const glm::mat4& view, const glm::mat4& projection, GLfloat deltaTime) const
{
    waterShader_->Use();
    glBindVertexArray(skyboxVAO_);

    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(waterShader_->Program(), "model");
    GLint viewLoc = glGetUniformLocation(waterShader_->Program(), "view");
    GLint projLoc = glGetUniformLocation(waterShader_->Program(), "projection");

    // Pass the matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(worldModel));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint xShiftLoc = glGetUniformLocation(waterShader_->Program(), "xShift");
    GLint yShiftLoc = glGetUniformLocation(waterShader_->Program(), "yShift");

    static GLfloat xShift = 0;
    static GLfloat yShift = 0;

    xShift += deltaTime * waveSpeed_;
    yShift += deltaTime * waveSpeed_ * 0.8f;

    std::cout << sinf(xShift) << std::endl;
    glUniform1f(xShiftLoc, waveScale_ * sinf(xShift));
    glUniform1f(yShiftLoc, waveScale_ * cosf(yShift));

    glBindTexture(GL_TEXTURE_2D, waterTexture_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glDrawArrays(GL_TRIANGLES, 5 * 6, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

GLuint TerrainEngine::LoadTexture(const char* src)
{
    auto res = SOIL_load_OGL_texture(
        src,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    // to fix SOIL not unbind texture after loading flaw
    glBindTexture(GL_TEXTURE_2D, 0);
    return res;
}

} /* namespace cg */
