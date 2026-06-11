#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <list>
#include <iostream>
#include <filesystem>

class SoundManager {
public:
    static SoundManager& getInstance() {
        static SoundManager instance;
        return instance;
    }

    // wczytuje dźwięk z pliku
    void loadSound(const std::string& name, const std::string& filepath);
    
    // gra wczytany dźwięk
    void playSound(const std::string& name, float volume = 100.f);
    
    // czyści skończone dźwięki
    void update();
    
    // funkcja do znajdywania plików
    std::string getSoundPath(const std::string& filename) const;

    // metody do muzyki
    void playMusic(const std::string& filename, bool loop = true, float volume = 50.f);
    void playGameMusic();
    void stopMusic();
    void setMusicVolume(float volume);

    void loadAllSounds();

private:
    SoundManager() = default;
    ~SoundManager() = default;
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::map<std::string, sf::SoundBuffer> buffers;
    std::list<sf::Sound> activeSounds;
    sf::Music bgMusic;

    bool isPlayingGameMusic = false;
    int currentGameMusicIndex = -1;
    std::vector<std::string> gameMusicTracks;
};
