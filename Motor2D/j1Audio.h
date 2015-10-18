#ifndef __j1AUDIO_H__
#define __j1AUDIO_H__

#include "j1Module.h"

#include "SDL/include/SDL.h"
#include "SDL_mixer\include\SDL_mixer.h"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;
struct Mix_Chunk;

class j1Audio : public j1Module
{
public:

	j1Audio();

	// Destructor
	virtual ~j1Audio();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	// Play a music file
	bool j1Audio::PlayMusic(SDL_RWops* _music, const char* path, float fade_time = DEFAULT_MUSIC_FADE_TIME);

	_Mix_Music* Load_music(SDL_RWops* rwops);

	// Load a WAV in memory
	unsigned int LoadFx(const char* path);

	// Play a previously loaded WAV
	bool PlayFx(unsigned int fx, int repeat = 0);

	int getVolume();

	int SetVolume(int);

	int getVolumeIncrement();

	int SetVolumeIncrement(int);

	//Volume Control
	bool SetVolumeMusic(int i);

	bool LoadData(pugi::xml_node&);

	bool SaveData(pugi::xml_node&);

private:

	_Mix_Music*			music;
	p2List<Mix_Chunk*>	fx;
	int volume;
	int volumeIncrement;
};

#endif // __j1AUDIO_H__