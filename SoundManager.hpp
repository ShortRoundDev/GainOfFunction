#pragma once

#include <vector>
#include <memory>
#include <map>
#include <string>

#include "AL/al.h"
#include "AL/alc.h"
#include "ogg/ogg.h"

#include "glm/glm.hpp"

class SoundManager {
public:
    static void init();
    static std::unique_ptr<SoundManager> instance;
    
    static void update();

    SoundManager();
    ~SoundManager();

    ALuint whackSounds[2];
    ALuint whooshSound;
    ALuint bulletHitSound[2];
    ALuint fleshSound;

    ALuint ghoulPain[2];
    ALuint ghoulDeath[2];
    ALuint ghoulMoan[2];

    std::vector<ALuint> sourceStack;
    
    ALCdevice* device;
    ALCcontext* context;
    
    ALuint generateSource(ALuint* source, bool loop, float gain);
    ALuint loadFile(std::string path, bool stereo);
    int readOgg(std::string path, ALuint buffer, bool stereo);
    
    void playSound(std::string path, const glm::vec3& pos, float gain);
    void playSound(ALuint buffer, const glm::vec3& pos, float gain);
    ALuint playLoop(ALuint buffer, const glm::vec3& pos, float gain);

    std::map<std::string, ALuint> sounds;
    
    void _update();
};