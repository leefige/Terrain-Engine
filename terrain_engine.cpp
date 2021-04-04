#include "terrain_engine.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2.h>

namespace cg
{

GLfloat TerrainEngine::cubeVertices[] = {
    // x     y     z    texture coordinates
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

TerrainEngine::TerrainEngine() :
    heightmap_(nullptr), mapHeight_(0), mapWidth_(0), mapChannels_(0),
    waterTexture_(0), landTexture_(0), detailTexture_(0),
    skyboxShader_(nullptr),
    skyboxTextures_{0}
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

void TerrainEngine::DrawSkybox(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const
{
    skyboxShader_->Use();
    glBindVertexArray(skyboxVAO_);

    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(skyboxShader_->Program(), "model");
    GLint viewLoc = glGetUniformLocation(skyboxShader_->Program(), "view");
    GLint projLoc = glGetUniformLocation(skyboxShader_->Program(), "projection");

    // Pass the matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    for (int i = 0; i < 5; i++) {
        glBindTexture(GL_TEXTURE_2D, skyboxTextures_[i]);
        glDrawArrays(GL_TRIANGLES, i * 6, (i + 1) * 6);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
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
