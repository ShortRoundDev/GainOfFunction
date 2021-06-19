#include "SoundManager.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#include "GameManager.hpp"

std::unique_ptr<SoundManager> SoundManager::instance = nullptr;

void SoundManager::init() {
    instance = std::make_unique<SoundManager>();
    
}

SoundManager::SoundManager() : sounds(), sourceStack(), context(NULL) {
    device = alcOpenDevice(NULL);
    if(!device) {
        std::cerr << "Couldn't open AL Device!" << std::endl;
        return;
    }
    
    context = alcCreateContext(device, NULL);
    if(!alcMakeContextCurrent(context)) {
        std::cerr << "Couldn't create AL Context!" << std::endl;
        return;
    }
        
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    alListener3f(AL_POSITION, 0, 0, 0);
    ALfloat orientation[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, orientation);
    
    auto err = alGetError();
    if(err != AL_NO_ERROR){
        std::cerr << err << std::endl;
    }
    
    loadFile("Resources/Audio/pistol.ogg", true);
    loadFile("Resources/Audio/rifle.ogg", true);
    loadFile("Resources/Audio/click57.ogg", true);
    loadFile("Resources/Audio/ammo_pickup.ogg", false);
    loadFile("Resources/Audio/terminal_05.ogg", true);
    loadFile("Resources/Audio/doorOpen.ogg", false);
    loadFile("Resources/Audio/pickupRifle.ogg", true);
    loadFile("Resources/Audio/pickupPistol.ogg", true);
    loadFile("Resources/Audio/bubble.ogg", false);
    loadFile("Resources/Audio/Basement.ogg", true);
    loadFile("Resources/Audio/WeirdNoises.ogg", false);
}

SoundManager::~SoundManager() {
}

void SoundManager::update(){
    instance->_update();
}

void SoundManager::_update(){
    if (GameManager::state == PLAYING) {
        auto pos = GameManager::instance->player.pos;
        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
        auto up = GameManager::instance->camera.cameraUp;
        auto front = GameManager::instance->camera.cameraFront;
        float orientation[] = { front.x, front.y, front.z, up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, orientation);
    }
    for (auto source : sourceStack) {
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            alDeleteSources(1, &source);
            auto location = std::find(sourceStack.begin(), sourceStack.end(), source);
            if (location != sourceStack.end()) {
                sourceStack.erase(location);
            }
        }
    }
}

ALuint SoundManager::generateSource(ALuint* source, bool loop, float gain) {
    alGenSources(1, source);
    alSourcef(*source, AL_PITCH, 1.0f);
    alSourcef(*source, AL_MAX_GAIN, 5.0f);
    alSourcef(*source, AL_GAIN, gain);
    alSource3f(*source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    auto err = alGetError();
    if(err != AL_NO_ERROR){
        std::cerr << err << std::endl;
    }

    alSource3f(*source, AL_VELOCITY, 1.0f, 0, 0);
    err = alGetError();
    if(err != AL_NO_ERROR){
        std::cerr << err << std::endl;
    }
    
    alSourcef(*source, AL_ROLLOFF_FACTOR, 3.0f);
    err = alGetError();
    if(err != AL_NO_ERROR){
        std::cerr << err << std::endl;
    }
    
    alSourcef(*source, AL_REFERENCE_DISTANCE, 1.5f);
    
    alSourcef(*source, AL_MAX_DISTANCE, 10.0f);
    err = alGetError();
    if(err != AL_NO_ERROR){
        std::cerr << err << std::endl;
    }
    
    alSourcei(*source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    err = alGetError();
    if(err != AL_NO_ERROR){
        std::cerr << err << std::endl;
    }
    if(!loop)
        sourceStack.push_back(*source);
    
    return *source;
}

ALuint SoundManager::loadFile(std::string path, bool stereo) {
    ALuint buffer;
    alGenBuffers(1, &buffer);
    if(!readOgg(path, buffer, stereo)){
        sounds[path] = buffer;
    } else {
        std::cout << "Failed to read file " << path << std::endl;
    }
    
    return buffer;
}

int SoundManager::readOgg(std::string path, ALuint buffer, bool stereo) {
    FILE* fp = fopen(path.c_str(), "rb");
    if(!fp) {
        std::cerr << "Couldn't open OGG file " << path << std::endl;
        return -1;
    }

    OggVorbis_File vf;
    if(ov_open_callbacks(fp, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0){
        std::cerr << path << " is not a valid OGG file" << std::endl;
        fclose(fp);
        return -2;
    }
    
    vorbis_info* vi = ov_info(&vf, -1);
    ALenum format = stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    
    size_t length = ov_pcm_total(&vf, -1) * vi->channels * 2;
    short* pcmout = (short*)malloc(length);
    int status = 0;
    for(
        size_t size = 0, offset = 0, sel = 0;
        (size = ov_read(&vf, (char*) pcmout + offset, 4096, 0, 2, 1, (int*)&sel)) != 0;
        offset += size
    ){
        if(size <= 0) {
            std::cerr << "Bad OGG File!" << std::endl;
            status = -3;
            goto exit;
        }
    }
    alBufferData(buffer, format, pcmout, (ALsizei)length, vi->rate);
exit:
    free(pcmout);
    ov_clear(&vf);
    fclose(fp);
    return status;
}


void SoundManager::playSound(std::string path, const glm::vec3& pos, float gain) {
    auto bufferSpot = sounds.find(path);
    if(bufferSpot != sounds.end()){
        playSound(bufferSpot->second, pos, gain);
    }
}

void SoundManager::playSound(ALuint buffer, const glm::vec3& pos, float gain) {
    ALuint source;
    generateSource(&source, false, gain);
    alSourcei(source, AL_BUFFER, buffer);
    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
    alSourcePlay(source);
}

ALuint SoundManager::playLoop(ALuint buffer, const glm::vec3& pos, float gain) {
    ALuint source;
    generateSource(&source, true, gain);
    alSourcei(source, AL_BUFFER, buffer);
    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
    alSourcePlay(source);
    return source;
}