#include "p2Defs.h"
#include "p2Log.h"
#include <sstream> 

#include "j1Window.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Scene.h"
#include "j1FileSystem.h"
#include "j1App.h"


#include "PugiXml\src\pugixml.hpp"

// Constructor
j1App::j1App(int _argc, char* _args[])
{
	frames = 0;
	argc = _argc;
	args = _args;

	input = new j1Input();
	win = new j1Window();
	render = new j1Render();
	tex = new j1Textures();
	audio = new j1Audio();
	scene = new j1Scene();
	fs = new j1FileSystem();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(input);
	AddModule(win);
	AddModule(tex);
	AddModule(audio);
	AddModule(fs);
	AddModule(scene);

	// render last to swap buffer
	AddModule(render);
}

// Destructor
j1App::~j1App()
{
	// release modules
	p2List_item<j1Module*>* item = modules.end;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->prev;
	}

	modules.clear();

	config_file.reset();
}

void j1App::AddModule(j1Module* module)
{
	module->Init();
	modules.add(module);
}

// Called before render is available
bool j1App::Awake()
{
	bool ret = LoadConfig();

	p2List_item<j1Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		
		ret = item->data->Awake(config.child(item->data->name.GetString()));
		item = item->next;
	}

	return ret;
}

bool j1App::LoadConfig()
{
	bool ret = true;

	char* buf;
	int size = App->fs->Load("config.xml", &buf);
	pugi::xml_parse_result result = config_file.load_buffer(buf, size);
//	RELEASE(buf);

	if (result == NULL)
	{
		LOG("Could not load map xml file config.xml. pugi error: %s", result.description());
		ret = false;
	}
	else
	{
		config = config_file.child("config");
		app_config = config.child("app");
	}

	return ret;
}

// Called before the first frame
bool j1App::Start()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		ret = item->data->Start();
		item = item->next;
	}

	return ret;
}

// Called each loop iteration
bool j1App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT) == true)
		ret = false;

	if(ret == true)
		ret = PreUpdate();

	if(ret == true)
		ret = DoUpdate();

	if(ret == true)
		ret = PostUpdate();

	FinishUpdate();
	return ret;
}

// ---------------------------------------------
void j1App::PrepareUpdate()
{
}

// ---------------------------------------------
void j1App::FinishUpdate()
{
	if (want_to_save)
		SaveGameNow();

	if (want_to_load)
		LoadGameNow();
}

// Call modules before each loop iteration
bool j1App::PreUpdate()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PreUpdate();
	}

	return ret;
}

// Call modules on each loop iteration
bool j1App::DoUpdate()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->Update(dt);
	}

	return ret;
}

// Call modules after each loop iteration
bool j1App::PostUpdate()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PostUpdate();
	}

	return ret;
}

// Called before quitting
bool j1App::CleanUp()
{
	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.end;

	while(item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}

	return ret;
}

// ---------------------------------------
int j1App::GetArgc() const
{
	return argc;
}

// ---------------------------------------
const char* j1App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}



void j1App::LoadWarning()
{
	want_to_load = true;
}


void j1App::SaveWarning()
{
	want_to_save = true;
}


bool j1App::LoadGameNow()
{
	want_to_load = false;
	
	return true;
}
bool j1App::SaveGameNow()
{
	want_to_save = false;

	bool ret = true;

	char* buf;

		int size = App->fs->Load("Partida.xml", &buf);
		
		if (size != 0)
		{
			pugi::xml_parse_result result = saveData.load_buffer(buf, size);

			RELEASE(buf);

			if (result == NULL)
			{
				LOG("Could not load map xml file Partida.xml. pugi error: %s", result.description());
				ret = false;
			}
			else
			{
				gameData = &saveData.child("GameData");
			}

		}
		else
		{
			save_game.create("Partida.xml");
			saveData.append_child("GameData");

			p2List_item<j1Module*>* item;
			item = modules.start;

			while (item != NULL && ret == true) 
			{
				ret = item->data->SaveData(saveData.append_child(item->data->name.GetString()));//We call SaveData in every module
				item = item->next;
			}

			if (ret == true)
			{
				std::stringstream stream;
				saveData.save(stream);

				// we are done, so write data to disk
				fs->Save(save_game.GetString(), stream.str().c_str(), stream.str().length());
				LOG("... finished saving", save_game.GetString());
			}
			else
				LOG("Save process halted from an error in module %s", (item != NULL) ? item->data->name.GetString() : "unknown");

			

		}

	return true;

}