#include "p2Defs.h"
#include "j1App.h"
#include "p2Log.h"
#include "j1FileSystem.h"
#include "PhysFS/include/physfs.h"
#include "SDL/include/SDL.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

j1FileSystem::j1FileSystem() : j1Module()
{
	name.create("file_system");


	char* base_path = SDL_GetBasePath();
	PHYSFS_init(base_path);
	SDL_free(base_path);
	LOG("%s", SDL_GetBasePath());

	AddPath(".");

	SetWriteDir();

}

// Destructor
j1FileSystem::~j1FileSystem()
{
	PHYSFS_deinit();
}

bool j1FileSystem::Awake(pugi::xml_node& fileSystemData)
{
	bool ret = true;

	AddPath(fileSystemData.child("path").attribute("value").as_string());//We pass the path of the zip we want to load

	

	showSearchPath();

	return ret;

}

// Called before quitting
bool j1FileSystem::CleanUp()
{
	//LOG("Freeing File System subsystem");

	return true;
}

bool j1FileSystem::AddPath(const char* path)
{
	bool ret = false;

	if (!PHYSFS_mount(path, NULL, 1))
		LOG("Failed Adding path %s", PHYSFS_getLastError());
	else
		ret = true;

	return ret;
}


bool j1FileSystem::SetWriteDir()
{
	bool ret = true;
	if (!PHYSFS_setWriteDir(SDL_GetBasePath()))
		ret = false, LOG("Error setting the Write Dir.");

	AddPath(PHYSFS_getWriteDir());
	LOG("Write dir: %s", PHYSFS_getWriteDir());

	return ret;
}

void j1FileSystem::showSearchPath()
{
	char **i;

	for (i = PHYSFS_getSearchPath(); *i != NULL; i++)
		LOG("[%s] is in the search path.\n", *i);

}


// Read a whole file and put it in a new buffer
unsigned int j1FileSystem::Load(const char* file, char** buffer) const
{
	unsigned int ret = 0;

	PHYSFS_file* fs_file = PHYSFS_openRead(file);

	if(fs_file != NULL)
	{
		PHYSFS_sint32 size = PHYSFS_fileLength(fs_file);

		if(size > 0)
		{
			*buffer = new char[size];
			int readed = PHYSFS_read(fs_file, *buffer, 1, size);
			if(readed != size)
			{
				LOG("File System error while reading from file %s: %s\n", file, PHYSFS_getLastError());
				RELEASE(buffer);
			}
			else
				ret = readed;
		}

		if(PHYSFS_close(fs_file) == 0)
			LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
	}
	else
		LOG("File System error while opening file %s: %s\n", file, PHYSFS_getLastError());

	

	

	return ret;
}

// Read a whole file and put it in a new buffer
SDL_RWops* j1FileSystem::Load(const char* file) const
{
	char* buffer;
	int size = Load(file, &buffer);

	if(size > 0)
	{
		SDL_RWops* r = SDL_RWFromConstMem(buffer, size);
		if(r != NULL)
			r->close = close_sdl_rwops;

		return r;
	}
	else
		return NULL;
}

int close_sdl_rwops(SDL_RWops *rw)
{
	RELEASE(rw->hidden.mem.base);
	SDL_FreeRW(rw);
	return 0;
}

bool j1FileSystem::doesFileExist(const char* file)const
{
	return PHYSFS_exists(file) ? true : false;
}


SDL_RWops* j1FileSystem::LoadFile(const char* file, char* myBuff, PHYSFS_sint64* size)
{


	if (doesFileExist(file))
	{

		PHYSFS_file* myfile = PHYSFS_openRead(file);

		if (myfile != NULL)
		{
			PHYSFS_sint64 file_size = PHYSFS_fileLength(myfile);

			if (file_size > 0)
			{
				myBuff = new char[file_size];
				PHYSFS_read(myfile, myBuff, 1, file_size);

				PHYSFS_close(myfile);

				SDL_RWops* r = SDL_RWFromConstMem(myBuff, file_size);

				if (r != NULL)
					r->close = close_sdl_rwops;

				size = &file_size;

				return r;
			}
			else
			{
				LOG("File System error : ", file, PHYSFS_getLastError());
			}
		}
	}

	return NULL;

}





// Save a whole buffer to disk
unsigned int j1FileSystem::Save(const char* file, const char* buffer, unsigned int size)const
{
	unsigned int ret = 0;


	if (!PHYSFS_delete(file))
	{
		LOG("FILE NOT FOUND");
	}

		PHYSFS_file* fs_file = PHYSFS_openWrite(file);

		if (fs_file != NULL)
		{
			unsigned int written = PHYSFS_write(fs_file, (const void*)buffer, 1, size);
			if (written != size)
				LOG("File System error while writing to file %s: %s\n", file, PHYSFS_getLastError());
			else
				ret = written;

			if (PHYSFS_close(fs_file) == 0)
				LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
		}
		else
			LOG("File System error while opening file %s: %s\n", file, PHYSFS_getLastError());
	


	
	return ret;
}


void j1FileSystem::FileDelete()
{

	

}

