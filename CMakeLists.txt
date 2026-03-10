# CMakeList.txt : projet CMake pour CMakeProject1, incluez la source et définissez
# la logique spécifique au projet ici.
#
#cmake_minimum_required (VERSION 3.8)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
#if (POLICY CMP0141)
#  cmake_policy(SET CMP0141 NEW)
#  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
#endif()

#project ("CMakeProject1")

# Ajoutez une source à l'exécutable de ce projet.
#add_executable (CMakeProject1 "CMakeProject1.cpp" "CMakeProject1.h")

#if (CMAKE_VERSION VERSION_GREATER 3.12)
#  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
#endif()

# TODO: Ajoutez des tests et installez des cibles si nécessaire.


cmake_minimum_required(VERSION 3.21)
project(SystrayApp VERSION 1.0 LANGUAGES CXX)

# ── Standard C++17 ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Intégration vcpkg (via toolchain) ────────────────────────────────────────
# La toolchain vcpkg est passée via -DCMAKE_TOOLCHAIN_FILE ou via la
# variable d'environnement VCPKG_ROOT (définie plus haut dans le tuto).
# Rien d'autre à faire ici.

# ── Dépendances via vcpkg ─────────────────────────────────────────────────────
find_package(spdlog CONFIG REQUIRED)

#find_package(unofficial-simpleini CONFIG REQUIRED)

# ── Exécutable WIN32 (pas de console) ────────────────────────────────────────
add_executable(SystrayApp WIN32
    #src/main.cpp
    CMakeProject1.cpp
)

# ── Lien avec les bibliothèques nécessaires ───────────────────────────────────
target_link_libraries(SystrayApp
    PRIVATE
        spdlog::spdlog
        #unofficial::simpleini::simpleini
        shlwapi         # Shell_NotifyIcon, etc.
)

# ── Options de compilation selon le compilateur ──────────────────────────────
if (MSVC)
    target_compile_options(SystrayApp PRIVATE
        /W4             # niveau d'avertissement élevé
        /WX             # traiter les warnings comme des erreurs
        /utf-8          # source et exécution en UTF-8
        /permissive-    # conformité stricte au standard
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang-cl ou clang natif
    target_compile_options(SystrayApp PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
endif()

# ── Définitions préprocesseur Windows ────────────────────────────────────────
target_compile_definitions(SystrayApp PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0A00   # Windows 10+
)

# ── Informations de build à l'écran ──────────────────────────────────────────
message(STATUS "Compilateur : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type  : ${CMAKE_BUILD_TYPE}")
