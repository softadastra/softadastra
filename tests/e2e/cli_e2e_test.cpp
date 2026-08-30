#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#if defined(__linux__)
#include <sys/wait.h>
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
    }

    void TearDown() override
    {
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
    }

    int run(const std::string &arguments)
    {
      const auto output = root_ / "command-output.txt";

#if defined(_WIN32)
      const std::string command =
          "cd /d " + quote(project_) + " && set \"LOCALAPPDATA=" +
          state_.string() + "\" && set \"HOME=" + root_.string() +
          "\" && set \"PATH=" + path_ + "\" && " +
          quote(SOFTADASTRA_E2E_BINARY) + " " + arguments +
          " > " + quote(output) + " 2>&1";
#else
      const std::string command =
          "cd " + quote(project_) + " && XDG_STATE_HOME=" + quote(state_) +
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
  };

  TEST_F(CliE2eTest, RunsAProjectCommandWithArgumentsAndManagesItsLifecycle)
  {
    const auto fixture_copy = project_ / "fixture with spaces";
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
    EXPECT_NE(last_output_.find("Usage: softadastra run [name]"), std::string::npos);
  }
}
