// CMakeProject1.cpp : définit le point d'entrée de l'application.
//


#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS


#include "CMakeProject1.h"

/*using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	return 0;
}
*/

// src/main.cpp
// Application systray Win32 sans fenêtre principale
// Menu contextuel avec "À propos" et "Quitter"
/*
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
*/
#include <windows.h>
#include <shellapi.h>   // NOTIFYICONDATA, Shell_NotifyIcon

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <strsafe.h>

#include <SimpleIni.h>

#include <vector>
#include <string>
#include <locale>
#include <codecvt>

#include <iostream>
#include <sstream>


// ─── Constantes ──────────────────────────────────────────────────────────────
constexpr UINT WM_TRAYICON = WM_USER + 1;   // message personnalisé systray
constexpr UINT IDM_ABOUT = 1001;
constexpr UINT IDM_QUIT = 1002;
constexpr UINT TRAY_ICON_ID = 1;

// ─── Variables globales minimales ────────────────────────────────────────────
NOTIFYICONDATA  g_nid = {};
HMENU           g_hMenu = nullptr;
HWND            g_hWnd = nullptr;

struct SProgramme {
    std::string nom;
    std::string exec;
    std::string param;
    int noMenu;
};

std::vector<SProgramme> listeProgrammes;

// ─── Initialisation des logs (fichier + Output VS) ───────────────────────────
void InitLogger()
{
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "systray_app.log", /*truncate=*/true);
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    auto logger = std::make_shared<spdlog::logger>(
        "app",
        spdlog::sinks_init_list{ file_sink, msvc_sink });

    logger->set_level(spdlog::level::debug);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    logger->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(logger);

    spdlog::info("Logger initialisé.");
}

// ─── Ajout de l'icône dans le systray ────────────────────────────────────────
bool AddTrayIcon(HWND hWnd, HINSTANCE hInstance)
{
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = TRAY_ICON_ID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;

    // Icône : on charge depuis les ressources si disponible, sinon icône système
    g_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    if (!g_nid.hIcon)
        g_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"Mon Application Systray");

    if (!Shell_NotifyIcon(NIM_ADD, &g_nid))
    {
        spdlog::error("Shell_NotifyIcon(NIM_ADD) a échoué. Erreur : {}", GetLastError());
        return false;
    }
    spdlog::info("Icône systray ajoutée.");
    return true;
}

// ─── Suppression de l'icône ──────────────────────────────────────────────────
void RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    spdlog::info("Icône systray supprimée.");
}

// ─── Affichage du menu contextuel ────────────────────────────────────────────
void ShowContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);

    // Obligatoire pour que le menu disparaisse correctement
    SetForegroundWindow(hWnd);

    TrackPopupMenu(
        g_hMenu,
        TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_RIGHTALIGN,
        pt.x, pt.y,
        0, hWnd, nullptr);

    PostMessage(hWnd, WM_NULL, 0, 0);
}

bool ExecuteProgram(const std::wstring& exePath, const std::wstring& args = L"")
{
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);

    // La ligne de commande doit être modifiable (non const)
    std::wstring cmdLine = L"\"" + exePath + L"\" " + args;

    BOOL success = CreateProcessW(
        nullptr,          // nom de l'exe (dans cmdLine)
        cmdLine.data(),   // ligne de commande complète
        nullptr,          // attributs process (défaut)
        nullptr,          // attributs thread (défaut)
        FALSE,            // pas d'héritage des handles
        0,                // flags (voir variantes ci-dessous)
        nullptr,          // variables d'environnement (héritées)
        nullptr,          // répertoire de travail (hérite du parent)
        &si,
        &pi);

    if (!success)
    {
        spdlog::error("CreateProcess échoué. Erreur : {}", GetLastError());
        return false;
    }

    // Toujours fermer ces handles pour éviter les fuites
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    spdlog::info("Processus lancé. PID : {}", pi.dwProcessId);
    return true;
}

void traitementClique(int noMenu) {

    spdlog::info("debut traitementClique : {} {} {}", noMenu, IDM_QUIT, listeProgrammes.size());

    if (noMenu > IDM_QUIT&&noMenu<= (IDM_QUIT+listeProgrammes.size())) {

        int n = noMenu;

        spdlog::info("n : {}", n);

        for (auto programme : listeProgrammes) {

            spdlog::info("programme.noMenu : {}", programme.noMenu);

            if (programme.noMenu > 0 && n == programme.noMenu) {

                spdlog::info("programme trouve. no menu : {} ({})", programme.noMenu,programme.nom);

                spdlog::info("exec : {} ; {}", programme.exec, programme.param);

                //wstring exec = programme.exec;
                //wstring param = programme.param;

                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                //std::string narrow = converter.to_bytes(wide_utf16_source_string);
                std::wstring exec = converter.from_bytes(programme.exec);
                std::wstring param = converter.from_bytes(programme.param);

                bool res = ExecuteProgram(exec, param);

                spdlog::info("res : {}", res);

                break;
            }

        }
    }

    spdlog::info("fin traitementClique");

}

// ─── Procédure de la fenêtre cachée ──────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TRAYICON:
        // lParam contient l'événement souris sur l'icône
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
        {
            spdlog::debug("Clic droit sur l'icône systray.");
            ShowContextMenu(hWnd);
        }
        else if (lParam == WM_LBUTTONDBLCLK)
        {
            spdlog::debug("Double-clic gauche sur l'icône systray.");
            MessageBox(hWnd,
                L"Mon Application Systray\nVersion 1.0",
                L"À propos",
                MB_OK | MB_ICONINFORMATION);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_ABOUT:
            spdlog::info("Menu → À propos");
            MessageBox(hWnd,
                L"Mon Application Systray\nVersion 1.0",
                L"À propos",
                MB_OK | MB_ICONINFORMATION);
            break;

        case IDM_QUIT:
            spdlog::info("Menu → Quitter");
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        default:
            spdlog::info("traitementClique {}", LOWORD(wParam));
            traitementClique(LOWORD(wParam));
            break;
        }
        return 0;

    case WM_CLOSE:
        RemoveTrayIcon();
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

void initMenu() {

    AppendMenu(g_hMenu, MF_STRING, IDM_ABOUT, L"À propos");
    AppendMenu(g_hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(g_hMenu, MF_STRING, IDM_QUIT, L"Quitter");

    // Lire un fichier .ini
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile("config.ini");

    const char* value = ini.GetValue("section", "key", "valeur_defaut");
    spdlog::info("Valeur lue : {}", value);

    const char* value2 = ini.GetValue("global", "liste_sections", "");

    std::string s = value2;

    spdlog::info("liste_sections : {}", s);

    std::vector<std::string> strings;
    std::istringstream f(s);
    std::string s2;
    while (getline(f, s2, ',')) {
        //cout << s << endl;
        strings.push_back(s2);
    }

    //spdlog::info("strings : {}", strings);
    spdlog::info("strings.size() : {}", strings.size());

    for (std::string s3 : strings) {

        spdlog::info("s3 : {}", s3);

        const char* nom = ini.GetValue(s3.c_str(), "nom", "");
        const char* exec = ini.GetValue(s3.c_str(), "exec", "");
        const char* param = ini.GetValue(s3.c_str(), "param", "");
        
        spdlog::info("nom : {}", nom);
        spdlog::info("exec : {}", exec);
        spdlog::info("param : {}", param);

        std::string nom2 = nom;
        std::string exec2 = exec;
        std::string param2 = param;

        spdlog::info("nom2 : {}", nom2);
        spdlog::info("exec2 : {}", exec2);
        spdlog::info("param2 : {}", param2);

        if (nom2.length()>0 && exec2.length()>0) {

            struct SProgramme programme;
            programme.nom = nom2;
            programme.exec = exec2;
            programme.param = param2;
            programme.noMenu = 0;

            listeProgrammes.push_back(programme);
        }

    }

    spdlog::info("taille listeProgrammes : {}", listeProgrammes.size());

    int n = IDM_QUIT+1;

    for (SProgramme& programme : listeProgrammes) {
        const char *s4 = programme.nom.c_str();

        size_t size = strlen(s4);
        wchar_t* wArr = new wchar_t[size+2]();
        for (size_t i = 0; i < size; ++i) {
            wArr[i] = s4[i];
        }

        programme.noMenu = n;

        AppendMenu(g_hMenu, MF_STRING, n, wArr);

        n++;
    }


}

// ─── Point d'entrée Win32 (pas de console) ───────────────────────────────────
int WINAPI WinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_     LPSTR     /*lpCmdLine*/,
    _In_     int       /*nCmdShow*/)
{
    InitLogger();
    spdlog::info("Démarrage de l'application.");

    // ── Enregistrement de la classe de fenêtre (cachée) ──────────────────────
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SystrayWindowClass";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        spdlog::critical("RegisterClassEx a échoué. Erreur : {}", GetLastError());
        return 1;
    }

    // ── Création d'une fenêtre message-only (invisible, pas dans la taskbar) ─
    g_hWnd = CreateWindowEx(
        0,
        L"SystrayWindowClass",
        L"SystrayApp",
        0,                          // pas de style
        0, 0, 0, 0,                 // taille et position nulles
        HWND_MESSAGE,               // fenêtre message-only
        nullptr, hInstance, nullptr);

    if (!g_hWnd)
    {
        spdlog::critical("CreateWindowEx a échoué. Erreur : {}", GetLastError());
        return 1;
    }

    // ── Construction du menu contextuel ──────────────────────────────────────
    g_hMenu = CreatePopupMenu();
    //AppendMenu(g_hMenu, MF_STRING, IDM_ABOUT, L"À propos");
    //AppendMenu(g_hMenu, MF_SEPARATOR, 0, nullptr);
    //AppendMenu(g_hMenu, MF_STRING, IDM_QUIT, L"Quitter");
    initMenu();

    // ── Ajout de l'icône dans le systray ─────────────────────────────────────
    if (!AddTrayIcon(g_hWnd, hInstance))
        return 1;

    // ── Boucle de messages ────────────────────────────────────────────────────
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ── Nettoyage ─────────────────────────────────────────────────────────────
    DestroyMenu(g_hMenu);
    spdlog::info("Application terminée proprement.");
    spdlog::shutdown();

    return static_cast<int>(msg.wParam);
}
