#ifndef CG_TERRAIN_ENGINE_H_

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <trimesh2/TriMesh.h>

#include "shader.hpp"

namespace cg
{

class TerrainEngine
{
public:
	static constexpr glm::vec3 skyboxSize{80.0f, 43.0f, 80.0f};
	static constexpr glm::vec3 terrainSize{30.0f, 7.0f, 30.0f};

	static constexpr GLsizei cubeVertNum = 36;
	static constexpr GLsizei cubeAttrNum = 5;
	static const GLfloat cubeVertices[cubeVertNum * cubeAttrNum];

	static const glm::mat4 worldModel;
	static const glm::mat4 landModel;

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
	GLuint TerrainTexture(int idx) const { return terrainTextures_[idx]; }
	GLuint SkyboxTexture(int idx) const { return skyboxTextures_[idx]; }

	GLfloat WaveSpeed() const { return waveSpeed_; }
	GLfloat WaveScale() const { return waveScale_; }
	GLfloat WaterAlpha() const { return waterAlpha_; }

	/* Setters */
	void SetWaveSpeed(GLfloat newSpeed) { waveSpeed_ = newSpeed; }
	void SetWaveScale(GLfloat newScale) { waveScale_ = newScale; }
	void SetWaterAlpha(GLfloat newAlpha) { waterAlpha_ = newAlpha; }

	/* load images */
	bool LoadHeightmap(const char* heightmapFile);
	bool LoadSkybox(const char* const skyboxFiles[5]);
	bool LoadWaterTexture(const char* waterFile);
	bool LoadTerrainTexture(const char* landFile, const char* detailFile);

	/* load shaders */
	bool InstallSkyboxShaders(const char* vert, const char* frag);
	bool InstallWaterShaders(const char* vert, const char* frag);
	bool InstallTerrainShaders(const char* vert, const char* frag);

	/* drawing */
	void DrawSkybox(const glm::mat4& view, const glm::mat4& projection) const;
	void DrawWater(const glm::mat4& view, const glm::mat4& projection, GLfloat deltaTime) const;
	void DrawTerrain(const glm::mat4& view, const glm::mat4& projection) const;

private:
	GLfloat waveSpeed_;
	GLfloat waveScale_;
	GLfloat waterAlpha_;

	int mapWidth_;
	int mapHeight_; 
	int mapChannels_;
	unsigned char* heightmap_;
	trimesh::TriMesh terrain_;
	int terrainDrawSize_;

	GLuint skyboxVAO_;
	GLuint skyboxVBO_;

	GLuint terrainVAO_;
	GLuint terrainVBO_;

	GLuint waterTexture_;
	GLuint terrainTextures_[2];
	GLuint skyboxTextures_[5];

	std::unique_ptr<Shader> skyboxShader_;
	std::unique_ptr<Shader> waterShader_;
	std::unique_ptr<Shader> terrainShader_;

	GLuint LoadTexture(const char* src);
	void DrawSkybox(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const;
};

} /* namespace cg */

#endif /* CG_TERRAIN_ENGINE_H_ */

