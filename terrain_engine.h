#ifndef CG_TERRAIN_ENGINE_H_

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.hpp"

namespace cg
{

class TerrainEngine
{
public:
	static constexpr GLsizei cubeVertNum = 36;
	static constexpr GLsizei cubeAttrNum = 5;
	static const GLfloat cubeVertices[cubeVertNum * cubeAttrNum];
	static const glm::mat4 worldModel;

	TerrainEngine();

	// forbid copying
	TerrainEngine(const TerrainEngine&) = delete;
	TerrainEngine(TerrainEngine&&) = delete;
	TerrainEngine operator=(const TerrainEngine&) = delete;
	TerrainEngine operator=(TerrainEngine&&) = delete;

	virtual ~TerrainEngine();

	/* Getters */
	const unsigned char* const Heightmap() const { return heightmap_; }
	int HeightmapWidth() const { return mapWidth_; }
	int HeightmapHeight() const { return mapHeight_; }
	int HeightmapChannels() const { return mapChannels_; }
	GLuint WaterTexture() const { return waterTexture_; }
	GLuint LandTexture() const { return landTexture_; }
	GLuint DetailTexture() const { return detailTexture_; }
	GLuint SkyboxTexture(int idx) const { return skyboxTextures_[idx]; }

	GLfloat WaterSpeed() const { return waterSpeed_; }

	/* Setters */
	void SetWaterSpeed(GLfloat newSpeed) { waterSpeed_ = newSpeed; }

	/* load images */
	bool LoadHeightmap(const char* heightmapFile);
	bool LoadSkybox(const char* const skyboxFiles[5]);
	bool LoadWaterTexture(const char* waterFile);
	bool LoadLandTexture(const char* landFile);
	bool LoadDetailTexture(const char* detailFile);

	/* load shaders */
	bool InstallSkyboxShaders(const char* vert, const char* frag);
	bool InstallWaterShaders(const char* vert, const char* frag);

	/* drawing */
	void DrawSkybox(const glm::mat4& view, const glm::mat4& projection) const;
	void DrawWater(const glm::mat4& view, const glm::mat4& projection, GLfloat deltaTime) const;

private:
	GLfloat waterSpeed_;

	int mapWidth_;
	int mapHeight_; 
	int mapChannels_;
	unsigned char* heightmap_;

	GLuint skyboxVAO_;
	GLuint skyboxVBO_;

	GLuint skyboxTextures_[5];
	GLuint waterTexture_;
	GLuint landTexture_;
	GLuint detailTexture_;

	std::unique_ptr<Shader> skyboxShader_;
	std::unique_ptr<Shader> waterShader_;

	GLuint LoadTexture(const char* src);
};

} /* namespace cg */

#endif /* CG_TERRAIN_ENGINE_H_ */

