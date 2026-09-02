#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <system_error>
#include <thread>

#if defined(__linux__)
#include <sys/wait.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

namespace
{
  std::string quote(const std::filesystem::path &path)
  {
    std::string value = path.string();
    std::string result{"\""};

    for (const char character : value)
    {
      if (character == '\"')
      {
        result += "\\\"";
      }
      else
      {
        result += character;
      }
    }

    return result + "\"";
  }

  std::optional<std::filesystem::path> reported_path(
      const std::string &output,
      const std::string &heading)
  {
    const auto start = output.find(heading);

    if (start == std::string::npos)
    {
      return std::nullopt;
    }

    const auto path_start = start + heading.size();
    const auto path_end = output.find('\n', path_start);

    return std::filesystem::path(
        output.substr(path_start, path_end - path_start));
  }

  class CliE2eTest : public ::testing::Test
  {
  protected:
    void SetUp() override
    {
      root_ = std::filesystem::temp_directory_path() /
              ("softadastra-e2e-" +
               std::to_string(std::chrono::steady_clock::now()
                                  .time_since_epoch()
                                  .count()));
      project_ = root_ / "project";
      state_ = root_ / "state";
#if defined(_WIN32)
      char *path = nullptr;
      std::size_t path_size = 0;
      const errno_t path_result =
          ::_dupenv_s(&path, &path_size, "PATH");
      path_ = path_result == 0 && path != nullptr ? path : "";
      std::free(path);
#else
      const char *path = std::getenv("PATH");
      path_ = path == nullptr ? "" : path;
#endif
      std::filesystem::create_directories(project_);
      std::filesystem::create_directories(state_);

#if defined(_WIN32)
      previous_local_app_data_ = environment("LOCALAPPDATA");
      previous_home_ = environment("HOME");
      ASSERT_TRUE(::SetEnvironmentVariableA(
          "LOCALAPPDATA", state_.string().c_str()));
      ASSERT_TRUE(::SetEnvironmentVariableA("HOME", root_.string().c_str()));

      std::wstring command =
          L"\"" + std::filesystem::path(SOFTADASTRA_E2E_BINARY).wstring() +
          L"\" host";
      STARTUPINFOW startup{};
      startup.cb = sizeof(startup);
      PROCESS_INFORMATION process{};

      ASSERT_TRUE(::CreateProcessW(
          nullptr,
          command.data(),
          nullptr,
          nullptr,
          FALSE,
          CREATE_NEW_PROCESS_GROUP,
          nullptr,
          project_.c_str(),
          &startup,
          &process));
      ::CloseHandle(process.hThread);
      host_process_ = process.hProcess;
      host_pid_ = process.dwProcessId;
#endif
    }

    void TearDown() override
    {
#if defined(_WIN32)
      static_cast<void>(run("host stop"));

      if (host_process_ != nullptr)
      {
        if (::WaitForSingleObject(host_process_, 5000) == WAIT_TIMEOUT)
        {
          static_cast<void>(::TerminateProcess(host_process_, 1));
          static_cast<void>(::WaitForSingleObject(host_process_, INFINITE));
        }

        ::CloseHandle(host_process_);
        host_process_ = nullptr;
      }

      static_cast<void>(::SetEnvironmentVariableA(
          "LOCALAPPDATA",
          previous_local_app_data_.has_value()
              ? previous_local_app_data_->c_str()
              : nullptr));
      static_cast<void>(::SetEnvironmentVariableA(
          "HOME",
          previous_home_.has_value() ? previous_home_->c_str() : nullptr));
      std::filesystem::remove_all(root_);
      return;
#else
      bool stopped = false;

      for (int attempt = 0; attempt < 500; ++attempt)
      {
        static_cast<void>(run("host status"));

        if (last_output_.find("Host: stopped") != std::string::npos)
        {
          stopped = true;
          break;
        }

        if (last_output_.find("Host: running") != std::string::npos)
        {
          static_cast<void>(run("host stop"));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }

      if (!stopped)
      {
        ADD_FAILURE() << "Softadastra Host remained active during E2E cleanup";
        return;
      }

      std::filesystem::remove_all(root_);
#endif
    }

    int run(const std::string &arguments)
    {
      return run_from(project_, arguments);
    }

    int run_from(
        const std::filesystem::path &project,
        const std::string &arguments)
    {
      const auto output = root_ / "command-output.txt";

#if defined(_WIN32)
      const std::string command =
          "cd /d " + quote(project) + " && set \"LOCALAPPDATA=" +
          state_.string() + "\" && set \"HOME=" + root_.string() +
          "\" && set \"PATH=" + path_ + "\" && " +
          quote(SOFTADASTRA_E2E_BINARY) + " " + arguments +
          " > " + quote(output) + " 2>&1";
#else
      const std::string command =
          "cd " + quote(project) + " && XDG_STATE_HOME=" + quote(state_) +
          " HOME=" + quote(root_) + " PATH=" + quote(path_) + " " +
          quote(SOFTADASTRA_E2E_BINARY) +
          " " + arguments + " > " + quote(output) + " 2>&1";
#endif

      const int result = std::system(command.c_str());
      std::ifstream input(output);
      last_output_.assign(
          std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>());
      input.close();

#if defined(__linux__)
      return result == -1 ? result : WEXITSTATUS(result);
#else
      return result;
#endif
    }

    std::filesystem::path root_;
    std::filesystem::path project_;
    std::filesystem::path state_;
    std::string path_;
    std::string last_output_;

#if defined(_WIN32)
    static std::optional<std::string> environment(const char *name)
    {
      char *value = nullptr;
      std::size_t size = 0;

      if (::_dupenv_s(&value, &size, name) != 0 || value == nullptr)
      {
        return std::nullopt;
      }

      std::string result(value);
      std::free(value);
      return result;
    }

    HANDLE host_process_{nullptr};
    DWORD host_pid_{0};
    std::optional<std::string> previous_local_app_data_;
    std::optional<std::string> previous_home_;
#endif
  };

  TEST_F(CliE2eTest, RunsAProjectCommandWithArgumentsAndManagesItsLifecycle)
  {
    auto fixture_copy = project_ / "fixture with spaces";
    fixture_copy.replace_extension(
        std::filesystem::path(SOFTADASTRA_E2E_TEST_APP).extension());
    std::filesystem::copy_file(
        SOFTADASTRA_E2E_TEST_APP,
        fixture_copy,
        std::filesystem::copy_options::overwrite_existing);

    const std::string declared_command =
        quote(fixture_copy) +
        " --stay --stdout stdout-value --stderr \"stderr value\"";

    ASSERT_EQ(
        run("init demo --command " + quote(declared_command) +
            " --access http:8080"),
        0)
        << last_output_;
    EXPECT_TRUE(std::filesystem::exists(project_ / "softadastra.toml"));

    ASSERT_EQ(run("run"), 0) << last_output_;
    EXPECT_NE(last_output_.find("running: demo"), std::string::npos);
    ASSERT_EQ(run("status"), 0) << last_output_;
    EXPECT_NE(last_output_.find("demo: running"), std::string::npos);
    ASSERT_EQ(run("info"), 0) << last_output_;
    EXPECT_NE(last_output_.find("Name:       demo"), std::string::npos);
    EXPECT_NE(last_output_.find("State:      running"), std::string::npos);
    ASSERT_EQ(run("list --running"), 0) << last_output_;
    EXPECT_NE(last_output_.find("demo"), std::string::npos);

    ASSERT_EQ(run("logs"), 0) << last_output_;
    EXPECT_NE(last_output_.find("stdout-value"), std::string::npos);
    EXPECT_NE(last_output_.find("stderr value"), std::string::npos);
    ASSERT_EQ(run("logs --clear"), 0) << last_output_;
    ASSERT_EQ(run("logs"), 0) << last_output_;
    EXPECT_TRUE(last_output_.empty());

    ASSERT_EQ(run("stop"), 0) << last_output_;
    ASSERT_EQ(run("status"), 0) << last_output_;
    EXPECT_NE(last_output_.find("demo: stopped"), std::string::npos);
    ASSERT_EQ(run("start"), 0) << last_output_;
    ASSERT_EQ(run("restart"), 0) << last_output_;
    ASSERT_EQ(run("stop"), 0) << last_output_;
    ASSERT_EQ(run("remove"), 0) << last_output_;
    EXPECT_TRUE(std::filesystem::exists(project_ / "softadastra.toml"));

    EXPECT_NE(run("status"), 0);
    EXPECT_NE(last_output_.find("Software not found: demo"), std::string::npos);
  }

  TEST_F(CliE2eTest, ReportsUsefulFailuresWithoutChangingProjectFiles)
  {
    ASSERT_EQ(run("init failed --command \"does-not-exist --with arguments\""), 0)
        << last_output_;
    ASSERT_EQ(run("register failed -- does-not-exist --with arguments"), 0)
        << last_output_;
    ASSERT_NE(run("start failed"), 0);
    EXPECT_NE(last_output_.find("Failed to start software: failed"), std::string::npos);
    EXPECT_NE(last_output_.find("Reason:"), std::string::npos);
    EXPECT_NE(last_output_.find("command could not be started"), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(project_ / "softadastra.toml"));

    ASSERT_NE(run("stop missing"), 0);
    EXPECT_NE(last_output_.find("Software not found: missing"), std::string::npos);
    ASSERT_NE(run("run extra arguments"), 0);
    EXPECT_NE(last_output_.find("Usage:"), std::string::npos);
    EXPECT_NE(last_output_.find("softadastra run [name]"), std::string::npos);
  }

  TEST_F(CliE2eTest, ReportsTheCanonicalHostStateAndLifecycleResults)
  {
#if defined(_WIN32)
    ASSERT_EQ(run("host stop"), 0) << last_output_;
    EXPECT_NE(last_output_.find("stopped: host"), std::string::npos);
#endif

    ASSERT_EQ(run("host status"), 0) << last_output_;
    EXPECT_NE(last_output_.find("Host: stopped"), std::string::npos);

    ASSERT_EQ(run("host start"), 0) << last_output_;
    EXPECT_NE(last_output_.find("started: host"), std::string::npos);
    ASSERT_EQ(run("host start"), 0) << last_output_;
    EXPECT_NE(last_output_.find("already running: host"), std::string::npos);

    ASSERT_EQ(run("host status"), 0) << last_output_;
    EXPECT_NE(last_output_.find("Host: running"), std::string::npos);
    ASSERT_EQ(run("host info"), 0) << last_output_;
    EXPECT_NE(last_output_.find("State:          running"), std::string::npos);

    ASSERT_EQ(run("host stop"), 0) << last_output_;
    EXPECT_NE(last_output_.find("stopped: host"), std::string::npos);
    ASSERT_EQ(run("host stop"), 0) << last_output_;
    EXPECT_NE(last_output_.find("already stopped: host"), std::string::npos);

    ASSERT_EQ(run("host restart"), 0) << last_output_;
    EXPECT_NE(last_output_.find("restarted: host"), std::string::npos);
    ASSERT_EQ(run("host stop"), 0) << last_output_;
  }

  TEST_F(CliE2eTest, RejectsDifferentProjectsWithTheSameName)
  {
    const auto first_project = root_ / "project-a";
    const auto second_project = root_ / "project-b";
    std::filesystem::create_directories(first_project);
    std::filesystem::create_directories(second_project);

    auto fixture = first_project / "test-app";
    fixture.replace_extension(
        std::filesystem::path(SOFTADASTRA_E2E_TEST_APP).extension());
    std::filesystem::copy_file(
        SOFTADASTRA_E2E_TEST_APP,
        fixture,
        std::filesystem::copy_options::overwrite_existing);

    const std::string command = quote(fixture) + " --stay";

    ASSERT_EQ(
        run_from(
            first_project,
            "init duplicate --command " + quote(command)),
        0)
        << last_output_;
    ASSERT_EQ(run_from(first_project, "run"), 0) << last_output_;

    ASSERT_EQ(
        run_from(
            second_project,
            "init duplicate --command " + quote(command)),
        0)
        << last_output_;
    ASSERT_NE(run_from(second_project, "run"), 0);
    EXPECT_NE(
        last_output_.find("Software name already registered: duplicate"),
        std::string::npos);
    const auto existing_project = reported_path(
        last_output_,
        "Existing project:\n  ");
    const auto current_project = reported_path(
        last_output_,
        "Current project:\n  ");
    ASSERT_TRUE(existing_project.has_value());
    ASSERT_TRUE(current_project.has_value());

    std::error_code error;
    EXPECT_TRUE(std::filesystem::equivalent(
        existing_project.value(),
        first_project,
        error));
    EXPECT_FALSE(error);
    error.clear();
    EXPECT_TRUE(std::filesystem::equivalent(
        current_project.value(),
        second_project,
        error));
    EXPECT_FALSE(error);
    EXPECT_NE(
        last_output_.find(
            "Choose another name or remove the existing registration."),
        std::string::npos);
  }
}
