#define MINIAUDIO_IMPLEMENTATION

#include "sound.hpp"
#include <iostream>

Sound::Sound()
{
    ma_result result = ma_engine_init(nullptr, &engine); // Initialize the engine
    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio engine: " << ma_result_description(result) << std::endl;
    }
    else{
        initSoundLibrary();
        loopSound("menu");
    }
}

// plays the selected sound once
void Sound::playSound(const std::string &key)
{
    ma_sound_seek_to_pcm_frame(sounds[key], 0); // Seek to the beginning
    ma_sound_start(sounds[key]);                // Start playing the sound
}

void Sound::loopSound(const std::string &key)
{
    ma_sound_set_looping(sounds[key], true);
    ma_sound_seek_to_pcm_frame(sounds[key], 0); // Seek to the beginning
    ma_sound_start(sounds[key]);                // Start playing the sound
}

void Sound::stopSound(const std::string &key)
{
    if(ma_sound_is_playing(sounds[key]))
        ma_sound_stop(sounds[key]);
}

void Sound::stopAllSounds()
{
    for (auto &pair : sounds)
    {
        if (ma_sound_is_playing(pair.second))
            ma_sound_stop(pair.second); // Stop each sound
    }
}

// add a sound with a unique key to be played later
void Sound ::addSound(const std::string &key, const std::string &filename)
{
    auto *sound = new ma_sound();
    ma_result result = ma_sound_init_from_file(&engine, filename.c_str(), 0, nullptr, nullptr, sound);

    if (result == MA_SUCCESS)
    {
        sounds[key] = sound; // Store the sound in the map
    }
    else
    {
        std::cerr << "Error loading audio file: " << filename << std::endl;
        delete sound; // Clean up if initialization failed
    }
}

// add all sound files to the sound object
void Sound::initSoundLibrary() {
    addSound("menu", "assets/sound/menu.mp3");
    addSound("background", "assets/sound/background.mp3");

    addSound("ball_reflect", "assets/sound/ball_reflect.wav");
    addSound("mine_reflect", "assets/sound/mine_reflect.wav");
    addSound("ball_selfCollide", "assets/sound/ball_selfCollide.wav");
    addSound("player_drawWall", "assets/sound/player_drawWall.wav");

    addSound("player_explode", "assets/sound/player_explode.wav");
    addSound("player_deathYell", "assets/sound/player_deathYell.wav");
    addSound("player_winLaugh", "assets/sound/player_winLaugh.wav");
}

Sound::~Sound() = default;