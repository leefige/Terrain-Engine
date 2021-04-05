#include "terrain_engine.h"

#include <cmath>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>


namespace cg
{

const glm::mat4 TerrainEngine::worldModel = glm::translate(
    glm::scale(glm::mat4(1.0f), skyboxSize),
    glm::vec3(0.0f, 0.5f, 0.0f)
);

const glm::mat4 TerrainEngine::landModel = glm::translate(
    glm::scale(glm::mat4(1.0f), terrainSize),
    glm::vec3(-0.5f, -0.29f, -0.5f)
);

const GLfloat TerrainEngine::cubeVertices[] = {
    // x     y     z    texture coordinates
    // back
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    // right
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,

    // front
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,

    // left
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

    // top
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,

    // bottom: water
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
};

TerrainEngine::TerrainEngine() :
    heightmap_(nullptr), mapHeight_(0), mapWidth_(0), mapChannels_(0),
    waterTexture_(0), landTexture_(0), detailTexture_(0), skyboxTextures_{0},
    skyboxShader_(nullptr), waveSpeed_(0.3f), waveScale_(0.3f), waterAlpha_(0.75f)
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
    if (this->heightmap_ == nullptr) {
        return false;
    }

    for (int i = 0; i < mapHeight_; i++) {
        for (int j = 0; j < mapWidth_; j++) {
            int idx = i * mapWidth_ + j;
            terrain_.vertices.emplace_back(
                float(j) / mapWidth_,
                float(heightmap_[idx]) / 256,
                float(i) / mapHeight_
            );

            terrain_.grid.push_back(idx);
        }
    }

    terrain_.grid_height = mapHeight_;
    terrain_.grid_width = mapWidth_;

    terrain_.triangulate_grid(false);

    // VBO & VAO
    glGenVertexArrays(1, &terrainVAO_);
    glBindVertexArray(terrainVAO_);

    glGenBuffers(1, &terrainVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO_);

    // generate array
    std::vector<trimesh::point3> landVerts;
    for (const auto& f : terrain_.faces) {
        for (int i = 0; i < 3; i++) {
            landVerts.push_back(terrain_.vertices[f[i]]);
        }
    }

    terrainDrawSize_ = landVerts.size();

    glBufferData(GL_ARRAY_BUFFER, landVerts.size() * sizeof(trimesh::point3), &landVerts.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
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

bool TerrainEngine::InstallTerrainShaders(const char* vert, const char* frag)
{
    this->terrainShader_ = Shader::Create(vert, frag);
    return this->terrainShader_ != nullptr;
}

void TerrainEngine::DrawSkybox(const glm::mat4& view, const glm::mat4& projection) const
{
    DrawSkybox(worldModel, view, projection);
}

void TerrainEngine::DrawWater(const glm::mat4& view, const glm::mat4& projection, GLfloat deltaTime) const
{
    // draw a mirrorred sky
    const static glm::mat4 mirrorModel = glm::mat4({
        {1, 0, 0, 0},
        {0, -1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}}) * worldModel;

    DrawSkybox(mirrorModel, view, projection);

    // --------------------------------

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    // semi-transparent water
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    waterShader_->Use();
    glBindVertexArray(skyboxVAO_);

    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(skyboxShader_->Program(), "model");
    GLint viewLoc = glGetUniformLocation(skyboxShader_->Program(), "view");
    GLint projLoc = glGetUniformLocation(skyboxShader_->Program(), "projection");

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

    glUniform1f(xShiftLoc, waveScale_ * sinf(xShift));
    glUniform1f(yShiftLoc, waveScale_ * cosf(yShift));

    GLint alphaLoc = glGetUniformLocation(waterShader_->Program(), "waterAlpha");
    glUniform1f(alphaLoc, waterAlpha_);

    glBindTexture(GL_TEXTURE_2D, waterTexture_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glDrawArrays(GL_TRIANGLES, 5 * 6, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
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
        // erase border
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
}

void TerrainEngine::DrawTerrain(const glm::mat4& view, const glm::mat4& projection) const
{
    terrainShader_->Use();
    glBindVertexArray(terrainVAO_);

    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(terrainShader_->Program(), "model");
    GLint viewLoc = glGetUniformLocation(terrainShader_->Program(), "view");
    GLint projLoc = glGetUniformLocation(terrainShader_->Program(), "projection");

    // Pass the matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(landModel));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindTexture(GL_TEXTURE_2D, landTexture_);
    // erase border
    glDrawArrays(GL_TRIANGLES, 0, terrainDrawSize_);
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
