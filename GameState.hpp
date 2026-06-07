#pragma once

// stany gry pozwalające na zarządzanie główną pętlą
enum class GameState {
    menu,
    playing,
    paused,
    slot_selection,
    difficulty_selection
};
