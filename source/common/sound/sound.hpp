#pragma once


#include "miniaudio.hpp"
#include <string>        // To work with strings like filename
#include <unordered_map> // To manage multiple sounds
#include <memory>        // For smart pointers

// Class to manage multiple audio sounds
class Sound
{
    ma_engine engine; // The audio engine
    std::unordered_map<std::string, ma_sound *> sounds;  // Stores the sounds
public:

    Sound();
    ~Sound();

    void addSound(const std::string &key, const std::string &filename);
    void playSound(const std::string &key);
    void loopSound(const std::string &key);
    void stopSound(const std::string &key);
    void stopAllSounds();
    void initSoundLibrary();

};