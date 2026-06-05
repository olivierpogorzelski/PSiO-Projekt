cmake_minimum_required(VERSION 3.20)
project(WolfensteinClone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# zmiana wskazanie na folder konfiguracyjny wersji 2.5.1 zmień ścieżkę na taką gdzie wypakowałeś sfml 2.5.1
set(SFML_DIR "C:/SFML/SFML-2.5.1/lib/cmake/SFML")

# zmiana szukamy wersji 2.5 komponenty w wersjach dwójki często pisane są z małej litery
find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)

# TUTAJ: Dodajemy pliki .hpp obok plików .cpp
add_executable(WolfensteinClone
    main.cpp
    Game.cpp
    Game.hpp
    Map.cpp
    Map.hpp
    Player.cpp
    Player.hpp
    Renderer.cpp
    Renderer.hpp
    Enemy.cpp
    Enemy.hpp
    GameState.hpp
    Constants.hpp
    Item.hpp Item.cpp
    Projectile.hpp Projectile.cpp
)

# zmiana w sfml 2.5.1 targety nazywają się sfml-graphics z małych liter bez przedrostka sfml
target_link_libraries(WolfensteinClone PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)
