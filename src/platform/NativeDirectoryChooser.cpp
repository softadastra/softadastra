#include "platform/NativeDirectoryChooser.hpp"

#include <array>
#include <cstdio>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#else
#include <sys/wait.h>
#endif

namespace softadastra
{
  DirectoryChooserResult choose_project_directory()
  {
#if defined(_WIN32)
    const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    BROWSEINFOW browse{};
    browse.lpszTitle = L"Choose a project folder for Softadastra";
    browse.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    PIDLIST_ABSOLUTE selected = SHBrowseForFolderW(&browse);
    if (selected == nullptr) { if (SUCCEEDED(initialized)) CoUninitialize(); return {DirectoryChooserStatus::Cancelled, {}}; }
    std::array<wchar_t, MAX_PATH> path{};
    const bool resolved = SHGetPathFromIDListW(selected, path.data()) != FALSE;
    CoTaskMemFree(selected);
    if (SUCCEEDED(initialized)) CoUninitialize();
    return resolved ? DirectoryChooserResult{DirectoryChooserStatus::Selected, path.data()} : DirectoryChooserResult{};
#elif defined(__linux__)
    for (const char *command : {"zenity --file-selection --directory 2>/dev/null", "kdialog --getexistingdirectory 2>/dev/null"})
    {
      FILE *pipe = popen(command, "r");
      if (pipe == nullptr) continue;
      std::array<char, 4096> buffer{};
      std::string path;
      if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) path = buffer.data();
      const int result = pclose(pipe);
      if (!path.empty()) { if (path.back() == '\n') path.pop_back(); return {DirectoryChooserStatus::Selected, path}; }
      if (result != -1 && WIFEXITED(result) && WEXITSTATUS(result) != 127) return {DirectoryChooserStatus::Cancelled, {}};
    }
    return {DirectoryChooserStatus::Unavailable, {}};
#else
    return {DirectoryChooserStatus::Unavailable, {}};
#endif
  }
}
