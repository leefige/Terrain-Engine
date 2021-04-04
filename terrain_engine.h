#ifndef CG_TERRAIN_ENGINE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2.h>

namespace cg
{

class TerrainEngine
{
public:
	TerrainEngine() : 
		heightmap_(nullptr), mapHeight_(0), mapWidth_(0), mapChannels_(0),
		waterTexture_(0), landTexture_(0), detailTexture_(0),
		skyboxTextures_{0}
	{

	}

	// forbid copying
	TerrainEngine(const TerrainEngine&) = delete;
	TerrainEngine(TerrainEngine&&) = delete;
	TerrainEngine operator=(const TerrainEngine&) = delete;
	TerrainEngine operator=(TerrainEngine&&) = delete;

	virtual ~TerrainEngine() 
	{
		if (heightmap_ != nullptr) {
			SOIL_free_image_data(heightmap_);
		}

		glDeleteTextures(1, &waterTexture_);
		glDeleteTextures(1, &landTexture_);
		glDeleteTextures(1, &detailTexture_);
		glDeleteTextures(5, skyboxTextures_);
	}

	/* Getters */
	const unsigned char* const Heightmap() const { return heightmap_; }
	int HeightmapWidth() const { return mapWidth_; }
	int HeightmapHeight() const { return mapHeight_; }
	int HeightmapChannels() const { return mapChannels_; }
	GLuint WaterTexture() const { return waterTexture_; }
	GLuint LandTexture() const { return landTexture_; }
	GLuint DetailTexture() const { return detailTexture_; }
	GLuint SkyboxTexture(int idx) const { return skyboxTextures_[idx]; }

	/* load images */
	bool LoadHeightmap(const char* heightmapFile);
	bool LoadSkybox(const char* skyboxFiles[5]);
	bool LoadWaterTexture(const char* waterFile);
	bool LoadLandTexture(const char* landFile);
	bool LoadDetailTexture(const char* detailFile);

private:
	int mapWidth_;
	int mapHeight_; 
	int mapChannels_;
	unsigned char* heightmap_;

	GLuint skyboxTextures_[5];
	GLuint waterTexture_;
	GLuint landTexture_;
	GLuint detailTexture_;

	GLuint LoadTexture(const char* src);
};

} /* namespace cg */

#endif /* CG_TERRAIN_ENGINE_H_ */

