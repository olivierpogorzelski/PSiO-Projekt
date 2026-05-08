cmake_minimum_required(VERSION 3.20)
project(WolfensteinClone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. ZMIANA: Wskazanie na folder konfiguracyjny wersji 2.5.1 (Zmień ścieżkę na taką, gdzie wypakowałeś SFML 2.5.1)
set(SFML_DIR "C:/SFML/SFML-2.5.1/lib/cmake/SFML")

# 2. ZMIANA: Szukamy wersji 2.5 (komponenty w 2.x często pisane są z małej litery)
find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)

add_executable(WolfensteinClone main.cpp)

# 3. ZMIANA: W SFML 2.5.1 targety nazywają się sfml-graphics (z małych liter, bez przedrostka SFML::)
target_link_libraries(WolfensteinClone PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)