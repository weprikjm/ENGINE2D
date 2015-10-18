#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1FileSystem.h"
#include "j1Audio.h"

#include "SDL/include/SDL.h"
#include "SDL_mixer\include\SDL_mixer.h"
#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )

j1Audio::j1Audio() : j1Module()
{
	music = NULL;
	name.create("audio");
}

// Destructor
j1Audio::~j1Audio()
{}

// Called before render is available
bool j1Audio::Awake(pugi::xml_node& audioConfig)
{
	LOG("Loading Audio Mixer");
	bool ret = true;
	SDL_Init(0);


	

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_INIT_AUDIO could not initialize! SDL_Error: %s\n", SDL_GetError());
		active = false;
		ret = true;
	}

	// load support for the JPG and PNG image formats
	int flags = MIX_INIT_OGG;
	int init = Mix_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Mixer lib. Mix_Init: %s", Mix_GetError());
		active = false;
		ret = true;
	}

	//Initialize SDL_mixer
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		LOG("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		active = false;
		ret = true;
	}

	volumeIncrement = audioConfig.attribute("volumeIncrement").as_int();
	volume = audioConfig.attribute("volume").as_int();
	Mix_VolumeMusic(volume);

	return ret;
}

// Called before quitting
bool j1Audio::CleanUp()
{
	if(!active)
		return true;

	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	if(music != NULL)
	{
		Mix_FreeMusic(music);
	}

	p2List_item<Mix_Chunk*>* item;
	for(item = fx.start; item != NULL; item = item->next)
		Mix_FreeChunk(item->data);

	fx.clear();

	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return true;
}





bool j1Audio::PlayMusic(SDL_RWops* _music, const char* path, float fade_time)
{
	bool ret = true;

	if (_music != NULL)
	{
		if (fade_time > 0.0f)
		{
			Mix_FadeOutMusic(int(fade_time * 1000.0f));
		}
		else
		{
			Mix_HaltMusic();
		}

		// this call blocks until fade out is done

	}

	music = Load_music(_music);


	if (music == NULL)
	{
		LOG("Cannot load music %s. Mix_GetError(): %s\n", path, Mix_GetError());
		ret = false;
	}
	else
	{
		if (!fade_time > 0.0f)
		{
			if (Mix_FadeInMusic(music, -1, 10000) < 0)
			{
				LOG("Cannot fade in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}

		}
		else
		{
			if (Mix_PlayMusic(music, -1) < 0)
			{
				LOG("Cannot play in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
	}

	LOG("Successfully playing %s", path);
	return ret;
}

_Mix_Music* j1Audio::Load_music(SDL_RWops* rwops)
{
	_Mix_Music* song;
	song = Mix_LoadMUS_RW(rwops, 1);
	if (!song)
	{
		LOG("ERROR loading music from SDL_RWops");
	}



	return song;

}






// Load WAV
unsigned int j1Audio::LoadFx(const char* path)
{
	unsigned int ret = 0;

	if(!active)
		return 0;

	Mix_Chunk* chunk = Mix_LoadWAV_RW(App->fs->Load(path), 1);

	if(chunk == NULL)
	{
		LOG("Cannot load wav %s. Mix_GetError(): %s", path, Mix_GetError());
	}
	else
	{
		fx.add(chunk);
		ret = fx.count();
	}

	return ret;
}

// Play WAV
bool j1Audio::PlayFx(unsigned int id, int repeat)
{
	bool ret = false;

	if(!active)
		return false;

	if(id > 0 && id <= fx.count())
	{
		Mix_PlayChannel(-1, fx[id - 1], repeat);
	}

	return ret;
}

bool j1Audio::SetVolumeMusic(int i)
{
	bool ret = false;
	volume = Mix_VolumeMusic(i);

	if (volume < 0)
		volume = Mix_VolumeMusic(0);
	else if (volume > MIX_MAX_VOLUME)
		volume = Mix_VolumeMusic(MIX_MAX_VOLUME);
	
	
	
	

	return ret = true;
}

int j1Audio::getVolume(){ return volume; }

int j1Audio::SetVolume(int _volume){ return volume = _volume; }






int j1Audio::getVolumeIncrement(){ return volumeIncrement; }

int j1Audio::SetVolumeIncrement(int vol){ return volumeIncrement = vol; }






bool j1Audio::SaveData(pugi::xml_node& audioConfig)
{
	audioConfig.append_attribute("volume").set_value(volume);
	return true;
}


bool j1Audio::LoadData(pugi::xml_node& audioConfig)
{
	volume = audioConfig.attribute("volume").as_int();
	SetVolumeMusic(volume);

	return true;
}