/**
 *
 *  @file remote_access_config_test.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "control/RemoteAccessConfig.hpp"
TEST(RemoteAccessConfigTest, PersistsDisabledAndExplicitEnabledConfiguration)
{
  const auto path =
      std::filesystem::temp_directory_path() / "softadastra-remote-config-test" / "remote";
  std::filesystem::remove_all(path.parent_path());
  softadastra::RemoteAccessConfig config(path);
  EXPECT_TRUE(config.save({}));
  softadastra::RemoteAccessSettings disabled{true, "x", 1};
  EXPECT_TRUE(config.load(disabled));
  EXPECT_FALSE(disabled.enabled);
  EXPECT_TRUE(config.save({true, "127.0.0.1", 45678}));
  softadastra::RemoteAccessSettings enabled;
  EXPECT_TRUE(config.load(enabled));
  EXPECT_TRUE(enabled.enabled);
  EXPECT_EQ(enabled.address, "127.0.0.1");
  EXPECT_EQ(enabled.port, 45678);
  std::ofstream legacy(path, std::ios::trunc);
  legacy << "1 127.0.0.1 45678\n";
  legacy.close();
  EXPECT_FALSE(config.load(enabled));
  std::ifstream preserved(path);
  std::string contents((std::istreambuf_iterator<char>(preserved)), {});
  EXPECT_EQ(contents, "1 127.0.0.1 45678\n");
  std::filesystem::remove_all(path.parent_path());
}
