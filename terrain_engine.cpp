#include "terrain_engine.h"

bool cg::TerrainEngine::LoadHeightmap(const char* heightmapFile)
{
	this->heightmap_ = SOIL_load_image(
		heightmapFile,
		&this->mapWidth_, &this->mapHeight_, &this->mapChannels_,
		SOIL_LOAD_L
	);
	return this->heightmap_ != nullptr;
}

bool cg::TerrainEngine::LoadSkybox(const char* skyboxFiles[5])
{
	for (int i = 0; i < 5; i++) {
		if ((this->skyboxTextures_[i] = LoadTexture(skyboxFiles[i])) == 0) {
			return false;
		}
	}
	return true;
}

bool cg::TerrainEngine::LoadWaterTexture(const char* waterFile)
{
	return (this->waterTexture_ = LoadTexture(waterFile)) != 0;
}

bool cg::TerrainEngine::LoadLandTexture(const char* landFile)
{
	return (this->landTexture_ = LoadTexture(landFile)) != 0;
}

bool cg::TerrainEngine::LoadDetailTexture(const char* detailFile)
{
	return (this->detailTexture_ = LoadTexture(detailFile)) != 0;
}

GLuint cg::TerrainEngine::LoadTexture(const char* src)
{
	return SOIL_load_OGL_texture(
		src,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
}
