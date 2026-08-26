#ifndef SOFTADASTRA_PLATFORM_NATIVE_DIRECTORY_CHOOSER_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_DIRECTORY_CHOOSER_HPP

#include <filesystem>

namespace softadastra
{
  enum class DirectoryChooserStatus { Selected, Cancelled, Unavailable };

  struct DirectoryChooserResult
  {
    DirectoryChooserStatus status{DirectoryChooserStatus::Unavailable};
    std::filesystem::path path;
  };

  /** Opens the operating system's folder picker without adding a GUI toolkit. */
  [[nodiscard]] DirectoryChooserResult choose_project_directory();
}

#endif // SOFTADASTRA_PLATFORM_NATIVE_DIRECTORY_CHOOSER_HPP
