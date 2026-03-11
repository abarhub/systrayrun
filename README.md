# Systrayrun


Pour builder, il faut cloner :
```shell
cd C:\dev
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Ensuite, il faut déclarer la variable d'environnement `VCPKG_ROOT` pour qu'elle pointe vers le répertoire de vcpkg.


Pour générer les fichiers de build du projet :
```shell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
```

Pour builder :
```shell
cmake --build build --config Release
```

Pour executer :
```shell
.\build\Release\SystrayApp.exe
```

