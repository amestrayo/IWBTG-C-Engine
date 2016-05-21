#ifndef ASSETS_H
#define ASSETS_H

typedef struct Asset Asset;
typedef struct Asset
{
    char* name;
    Asset* next;
    Texture texture;
} Asset;

#define ASSET_ARRAY_SIZE 255
typedef struct
{
    Asset* assetArray[ASSET_ARRAY_SIZE];
} Assets;

Texture textureLoad(Game* game, char* filePath);
int assetsHash(char* string);
void assetsAddTexture(Assets* as, Game* g, Texture texture, char* name);
Texture* assetsGetTexture(Game* g, char* name);
void assetsLoadTexture(Game* g, char* filePath, char*name);

#endif // ASSETS_H