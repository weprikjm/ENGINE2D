#ifndef __j1MAP_H__
#define __j1MAP_H__

#include "PugiXml/src/pugixml.hpp"
#include "p2List.h"
#include "p2Point.h"
#include "j1Module.h"
#include "p2DynArray.h"
// TODO 2: Create a struct to hold information for a TileSet
// Ignore Terrain Types and Tile Types for now, but we want the image!


// TODO 1: Create a struct needed to hold the information to Map node

struct mapLayer
{
	int width;
	int height;
	p2DynArray<int> tileGuideline;//What tile goes where
};


enum orientation 
{
	ORTHOGONAL,
	ISOMETRIC,
};

enum renderOrder
{
	RIGHTDOWN,
	LEFTDOWN,
	RIGHTUP,
	LEFTUP
};

struct tileOffset
{
	int x;
	int y;

	tileOffset() :x(0), y(0){}
};

struct TileSet
{
	int firstGid;
	int tileWidth;
	int tileHeight;
	int spacing;
	int margin;
	int widthInTiles;
	int heightInTiles;

	p2DynArray<int> tileGrid;
	
	TileSet() : firstGid(0), tileWidth(0), tileHeight(0), spacing(0), margin(0){}

};


struct MapNode
{
	int width;
	int height;
	int tileWidth;
	int tileHeigth;
	int nextObjectId;
	int numTileSets;
	int	numLayers;
	int widthPx;
	int heightPx;

	tileOffset Offset;
	renderOrder render;
	orientation orient;

	p2List<TileSet*> tiles;
	p2List<mapLayer*> layers;

	MapNode() : width(0), height(0), tileWidth(0), tileHeigth(0), nextObjectId(0), 
		render(RIGHTDOWN), orient(ORTHOGONAL), numTileSets(0)
	{
	
		Offset.x = 0;
		Offset.y = 0;
	}
	
};





// ----------------------------------------------------
class j1Map : public j1Module
{
public:

	j1Map();

	// Destructor
	virtual ~j1Map();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called each loop iteration
	void Draw();

	// Called before quitting
	bool CleanUp();

	// Load new map
	bool Load(const char* path);
	
	MapNode* GetMapNode();

	SDL_Rect GetTileRect(int id);

	uint Get(int x, int y);

private:
	//Loads the data for the map configuration
	int LoadMapInfo();
	bool loadTileSet(int nTileSets);
	SDL_Texture* LoadRootTileGraphics(SDL_Texture*);
	bool LoadLayer(int);
	

public:

	
	MapNode Level1;

private:
	
	pugi::xml_document	map_file;
	p2SString			folder;
	bool				map_loaded;
	MapNode				map;
	SDL_Texture* gameArtTiles;

};

#endif // __j1MAP_H__