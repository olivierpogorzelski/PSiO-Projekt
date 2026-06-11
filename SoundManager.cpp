#include "SoundManager.hpp"
#include <iostream>

std::string SoundManager::getSoundPath(const std::string& filename) const {
    std::string paths[] = {
        "sounds/" + filename,
        "../sounds/" + filename,
        "../../sounds/" + filename,
        "../../../sounds/" + filename
    };
    for (const auto& p : paths) {
        if (std::filesystem::exists(p)) return p;
    }
    return "sounds/" + filename; // fallback
}

void SoundManager::loadSound(const std::string& name, const std::string& filepath) {
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(getSoundPath(filepath))) {
        buffers[name] = buffer;
    } else {
        std::cerr << "Failed to load sound: " << filepath << std::endl;
    }
}

void SoundManager::playSound(const std::string& name, float volume) {
    auto it = buffers.find(name);
    if (it != buffers.end()) {
        activeSounds.emplace_back(it->second);
        activeSounds.back().setVolume(volume);
        activeSounds.back().play();
    }
}

void SoundManager::update() {
    activeSounds.remove_if([](const sf::Sound& sound) {
        return sound.getStatus() == sf::Sound::Stopped;
    });

    if (isPlayingGameMusic && bgMusic.getStatus() == sf::SoundSource::Stopped) {
        // zagraj następną płytę
        currentGameMusicIndex++;
        if (currentGameMusicIndex >= gameMusicTracks.size()) {
            currentGameMusicIndex = 0;
        }
        if (!gameMusicTracks.empty()) {
            playMusic(gameMusicTracks[currentGameMusicIndex], false, 30.f);
            isPlayingGameMusic = true;
        }
    }
}

void SoundManager::playMusic(const std::string& filename, bool loop, float volume) {
    isPlayingGameMusic = false; // domyślnie zostaje nadpisane
    if (bgMusic.openFromFile(getSoundPath(filename))) {
        bgMusic.setLoop(loop);
        bgMusic.setVolume(volume);
        bgMusic.play();
    } else {
        std::cerr << "Failed to load music: " << filename << std::endl;
    }
}

void SoundManager::playGameMusic() {
    if (gameMusicTracks.empty()) {
        gameMusicTracks = {
            "music/Deep-Into-Code.ogg",
            "music/Facing-The-Spider.ogg",
            "music/Kitchen-Ace.ogg",
            "music/Mouth-For-War.ogg"
        };
    }
    
    // zaczyna od pierwszego lub następnego
    if (currentGameMusicIndex == -1) currentGameMusicIndex = 0;
    
    playMusic(gameMusicTracks[currentGameMusicIndex], false, 30.f); 
    isPlayingGameMusic = true; // ustawione na true, po pętli
}

void SoundManager::stopMusic() {
    bgMusic.stop();
}

void SoundManager::setMusicVolume(float volume) {
    bgMusic.setVolume(volume);
}

void SoundManager::loadAllSounds() {
    // drzwi
    loadSound("drzwi", "drzwi.WAV");

    // miecz
    for (int i = 1; i <= 10; ++i) {
        loadSound("miecz-cling" + std::to_string(i), "miecz-cling" + std::to_string(i) + ".WAV");
    }

    // strzała
    loadSound("strzala-wystrzal", "strzala-wystrzal.WAV");
    loadSound("strzala-trafienie", "strzala-trafienie.WAV");

    // gracz
    loadSound("gracz_smierc", "gracz/smierc.wav");
    loadSound("gracz_zraniony", "gracz/zraniony.wav");
    loadSound("gracz_potka", "gracz/potka.WAV");
    loadSound("gracz_podniesienie", "gracz/podniesienieprzedmiotu.wav");

    // duży demon
    loadSound("duzydemon_atakblisko", "duzydemon/atakblisko.wav");
    loadSound("duzydemon_atakdaleko", "duzydemon/atakdaleko.wav");
    loadSound("duzydemon_obok", "duzydemon/obok.wav");
    loadSound("duzydemon_smierc", "duzydemon/smierc.wav");
    loadSound("duzydemon_wzrok", "duzydemon/wzrok.wav");
    loadSound("duzydemon_zraniony", "duzydemon/zraniony.wav");

    // małe demony
    loadSound("malydemon_atakblisko", "malydemon/atakblisko.wav");
    loadSound("malydemon_atakdaleko", "malydemon/atakdaleko.wav");
    loadSound("malydemon_obok", "malydemon/obok.wav");
    loadSound("malydemon_smierc", "malydemon/smierc.wav");
    loadSound("malydemon_wzrok", "malydemon/wzrok.wav");
    loadSound("malydemon_zraniony", "malydemon/zraniony.wav");

    // goblin
    loadSound("goblin_topor1", "goblin/topor1.WAV");
    loadSound("goblin_topor2", "goblin/topor2.WAV");
    loadSound("goblin_topor3", "goblin/topor3.WAV");
    loadSound("goblin_obok", "goblin/obok.wav");
    loadSound("goblin_smierc", "goblin/smierc.wav");
    loadSound("goblin_wzrok", "goblin/wzrok.wav");
    loadSound("goblin_zraniony", "goblin/zraniony.wav");

    // szkielet
    loadSound("szkielet_obok", "szkielet/obok.wav");
    loadSound("szkielet_smierc", "szkielet/smierc.wav");
    loadSound("szkielet_wzrok", "szkielet/wzrok.wav");
    loadSound("szkielet_zraniony", "szkielet/zraniony.wav");

    // zjawa
    loadSound("zjawa_atak-magia", "zjawa/atak-magia.WAV");
    loadSound("zjawa_atak-magia2", "zjawa/atak-magia2.WAV");
    loadSound("zjawa_obok", "zjawa/obok.wav");
    loadSound("zjawa_smierc", "zjawa/smierc.wav");
    loadSound("zjawa_zraniony", "zjawa/zraniony.wav");
}
