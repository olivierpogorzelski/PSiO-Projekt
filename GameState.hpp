#pragma once
#pragma once

// stany gry pozwalające na zarządzanie główną petlą
enum class GameState {
    menu,
    playing,
    paused,
    slot_selection,
    difficulty_selection,
    gameOver,
    gameWon
};
