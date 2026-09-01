/**
 *
 *  @file WebUiServer.cpp
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

#include "webui/WebUiServer.hpp"

#include "WebUiAssets.hpp"
#include "host/HostObservation.hpp"
#include "platform/NativeDirectoryChooser.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "software/ProjectConfig.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <asio/write.hpp>

#include <filesystem>
#include <algorithm>
#include <istream>
#include <string_view>
#include <system_error>

#if defined(__linux__)

#include <fcntl.h>

#endif

namespace
{
  using Tcp = asio::ip::tcp;

  std::string response(
      int status,
      const std::string &body,
      const char *type = "application/json")
  {
    const char *reason =
        status == 200
            ? "OK"
        : status == 404
            ? "Not Found"
        : status == 503
            ? "Service Unavailable"
            : "Bad Request";

    return "HTTP/1.1 " +
           std::to_string(status) +
           " " +
           reason +
           "\r\nContent-Type: " +
           type +
           "\r\nContent-Length: " +
           std::to_string(body.size()) +
           "\r\nConnection: close\r\n\r\n" +
           body;
  }

  std::string escape_json(
      const std::string &value)
  {
    std::string escaped;

    for (char character : value)
    {
      if (character == '"' ||
          character == '\\')
      {
        escaped += '\\';
      }

      if (character == '\n')
      {
        escaped += "\\n";
      }
      else if (
          character != '\r' &&
          static_cast<unsigned char>(character) >= 0x20)
      {
        escaped += character;
      }
    }

    return escaped;
  }

  std::optional<std::string> url_decode(
      const std::string &value)
  {
    std::string decoded;
    decoded.reserve(value.size());

    for (std::size_t index = 0;
         index < value.size();
         ++index)
    {
      if (value[index] != '%')
      {
        decoded += value[index];
        continue;
      }

      if (index + 2 >= value.size())
      {
        return std::nullopt;
      }

      const auto hex =
          [](char character) -> int
      {
        if (character >= '0' &&
            character <= '9')
        {
          return character - '0';
        }

        if (character >= 'a' &&
            character <= 'f')
        {
          return character - 'a' + 10;
        }

        if (character >= 'A' &&
            character <= 'F')
        {
          return character - 'A' + 10;
        }

        return -1;
      };

      const int first =
          hex(value[index + 1]);

      const int second =
          hex(value[index + 2]);

      if (first < 0 ||
          second < 0)
      {
        return std::nullopt;
      }

      decoded +=
          static_cast<char>(
              (first << 4) |
              second);

      index += 2;
    }

    return decoded;
  }

  std::optional<std::uintmax_t> query_offset(
      std::string_view query)
  {
    constexpr std::string_view prefix =
        "offset=";

    if (!query.starts_with(prefix))
    {
      return std::nullopt;
    }

    const auto value =
        query.substr(prefix.size());

    if (value.empty() ||
        value.find('&') != std::string_view::npos)
    {
      return std::nullopt;
    }

    try
    {
      return static_cast<std::uintmax_t>(
          std::stoull(std::string(value)));
    }
    catch (const std::exception &)
    {
      return std::nullopt;
    }
  }

  std::optional<std::string> json_string(
      const std::string &body,
      const std::string &key)
  {
    const std::string marker =
        "\"" + key + "\"";

    const auto key_position =
        body.find(marker);

    if (key_position == std::string::npos)
    {
      return std::nullopt;
    }

    auto position =
        body.find(
            ':',
            key_position + marker.size());

    if (position == std::string::npos)
    {
      return std::nullopt;
    }

    while (++position < body.size() &&
           (body[position] == ' ' ||
            body[position] == '\t'))
    {
    }

    if (position >= body.size() ||
        body[position] != '\"')
    {
      return std::nullopt;
    }

    std::string value;

    for (++position;
         position < body.size();
         ++position)
    {
      const char character =
          body[position];

      if (character == '\"')
      {
        return value;
      }

      if (character != '\\')
      {
        value += character;
        continue;
      }

      if (++position >= body.size())
      {
        return std::nullopt;
      }

      const char escaped =
          body[position];

      if (escaped == 'n')
      {
        value += '\n';
      }
      else if (escaped == 'r')
      {
        value += '\r';
      }
      else if (escaped == 't')
      {
        value += '\t';
      }
      else if (
          escaped == '\"' ||
          escaped == '\\' ||
          escaped == '/')
      {
        value += escaped;
      }
      else
      {
        return std::nullopt;
      }
    }

    return std::nullopt;
  }

  std::optional<std::vector<softadastra::AccessPoint>>
  json_access_points(
      const std::string &body)
  {
    const auto key =
        body.find("\"access_points\"");

    if (key == std::string::npos)
    {
      return std::vector<softadastra::AccessPoint>{};
    }

    const auto begin =
        body.find(
            '[',
            key + 15);

    if (begin == std::string::npos)
    {
      return std::nullopt;
    }

    const auto end =
        body.find(
            ']',
            begin + 1);

    if (end == std::string::npos)
    {
      return std::nullopt;
    }

    std::vector<softadastra::AccessPoint> points;

    std::size_t object =
        body.find(
            '{',
            begin + 1);

    while (object != std::string::npos &&
           object < end)
    {
      const auto close =
          body.find(
              '}',
              object + 1);

      if (close == std::string::npos ||
          close > end)
      {
        return std::nullopt;
      }

      const auto endpoint =
          body.substr(
              object,
              close - object + 1);

      const auto protocol_name =
          json_string(
              endpoint,
              "protocol");

      const auto port_text =
          json_string(
              endpoint,
              "port");

      const auto protocol =
          protocol_name
              ? softadastra::AccessPoint::protocol(
                    *protocol_name)
              : std::nullopt;

      try
      {
        const auto port =
            port_text
                ? std::stoul(*port_text)
                : 0;

        const auto point =
            protocol &&
                    port > 0 &&
                    port <= 65535
                ? softadastra::AccessPoint::create(
                      *protocol,
                      static_cast<std::uint16_t>(port))
                : std::nullopt;

        if (!point)
        {
          return std::nullopt;
        }

        points.push_back(*point);
      }
      catch (const std::exception &)
      {
        return std::nullopt;
      }

      object =
          body.find(
              '{',
              close + 1);
    }

    return points;
  }

  std::string access_json(
      const softadastra::SoftwareEntry &entry,
      const std::vector<softadastra::LocalAccess> *local_accesses)
  {
    std::string result = "[";
    bool first = true;

    for (const auto &point : entry.access_points())
    {
      if (!first)
      {
        result += ',';
      }

      first = false;

      const auto protocol =
          std::string(
              softadastra::AccessPoint::name(
                  point.protocol()));

      result +=
          "{\"protocol\":\"" +
          protocol +
          "\",\"port\":" +
          std::to_string(point.port()) +
          ",\"configured\":\"" +
          protocol +
          ":" +
          std::to_string(point.port()) +
          "\",\"url\":\"";

      const auto access = local_accesses
          ? std::find_if(local_accesses->begin(), local_accesses->end(),
              [&point](const softadastra::LocalAccess &value)
              { return value.protocol == point.protocol() && value.port == point.port(); })
          : std::vector<softadastra::LocalAccess>::const_iterator{};
      if (local_accesses && access != local_accesses->end() &&
          access->state == softadastra::LocalAccessState::Available)
      {
        result += access->url;
      }

      result += "\"}";
    }

    return result + "]";
  }

  std::string command_name(
      const softadastra::SoftwareEntry &entry)
  {
    if (!entry.declared_command().empty())
    {
      return entry.declared_command();
    }

    const auto &spec =
        entry.process_spec();

    if ((spec.executable() == "/bin/sh" &&
         spec.arguments().size() == 2 &&
         spec.arguments()[0] == "-lc") ||
        (spec.executable() == "cmd.exe" &&
         spec.arguments().size() == 2 &&
         spec.arguments()[0] == "/C"))
    {
      return spec.arguments()[1];
    }

    return spec.executable();
  }

} // namespace

namespace softadastra
{
  WebUiServer::WebUiServer(
      ControlClient &client,
      ProjectDirectoryChooser directory_chooser) noexcept
      : client_(client),
        directory_chooser_(
            std::move(directory_chooser))
  {
  }

  WebUiServer::~WebUiServer()
  {
    stop();
  }

  void WebUiServer::publish_startup(
      bool success,
      std::uint16_t port) noexcept
  {
    std::lock_guard lock(
        startup_mutex_);

    startup_success_ = success;
    startup_complete_ = true;

    if (success)
    {
      port_.store(port);
    }

    startup_condition_.notify_one();
  }

  bool WebUiServer::start(
      std::uint16_t requested_port)
  {
    if (worker_.joinable())
    {
      return false;
    }

    stopping_.store(false);
    port_.store(0);

    {
      std::lock_guard lock(
          startup_mutex_);

      startup_complete_ = false;
      startup_success_ = false;
    }

    try
    {
      worker_ =
          std::thread(
              [this, requested_port]
              {
                run(requested_port);
              });
    }
    catch (...)
    {
      return false;
    }

    std::unique_lock lock(
        startup_mutex_);

    startup_condition_.wait(
        lock,
        [this]
        {
          return startup_complete_;
        });

    const bool success =
        startup_success_;

    lock.unlock();

    if (!success)
    {
      worker_.join();
      return false;
    }

    return true;
  }

  void WebUiServer::stop() noexcept
  {
    stopping_.store(true);

    const auto port =
        port_.load();

    if (worker_.joinable() &&
        port != 0)
    {
      // A loopback connection wakes the blocking accept on every supported
      // platform; the worker remains the sole owner of the accepted socket.
      asio::io_context context;
      Tcp::socket wake(context);
      std::error_code error;

      wake.connect(
          {asio::ip::make_address(
               "127.0.0.1",
               error),
           port},
          error);
    }

    if (worker_.joinable())
    {
      worker_.join();
    }

    port_.store(0);
  }

  void WebUiServer::run(
      std::uint16_t requested_port) noexcept
  {
    try
    {
      asio::io_context context;
      Tcp::acceptor acceptor(context);

      std::error_code error;

      const auto address =
          asio::ip::make_address(
              "127.0.0.1",
              error);

      acceptor.open(
          Tcp::v4(),
          error);

      acceptor.set_option(
          asio::socket_base::reuse_address(true),
          error);

      acceptor.bind(
          Tcp::endpoint(
              address,
              requested_port),
          error);

      acceptor.listen(
          asio::socket_base::max_listen_connections,
          error);

      if (error)
      {
        publish_startup(
            false,
            0);

        return;
      }

      publish_startup(
          true,
          acceptor.local_endpoint().port());

      while (!stopping_.load())
      {
        Tcp::socket socket(context);

        acceptor.accept(
            socket,
            error);

        if (error)
        {
          continue;
        }

        if (stopping_.load())
        {
          break;
        }

#if defined(__linux__)

        // Hosted commands are forked from this thread. They must not retain
        // an HTTP client connection and delay its EOF until they exit.
        static_cast<void>(
            ::fcntl(
                socket.native_handle(),
                F_SETFD,
                FD_CLOEXEC));

#endif

        asio::streambuf request;

        asio::read_until(
            socket,
            request,
            "\r\n\r\n",
            error);

        if (error)
        {
          continue;
        }

        std::istream input(&request);

        std::string method;
        std::string path;
        std::string version;

        input >> method >> path >> version;

        std::string query;

        if (const auto query_start =
                path.find('?');
            query_start != std::string::npos)
        {
          query =
              path.substr(
                  query_start + 1);

          path.resize(query_start);
        }

        std::string header;
        std::size_t content_length = 0;

        std::getline(
            input,
            header);

        while (std::getline(input, header) &&
               header != "\r")
        {
          constexpr std::string_view content_length_header =
              "Content-Length:";

          if (header.rfind(
                  content_length_header,
                  0) == 0)
          {
            try
            {
              content_length =
                  static_cast<std::size_t>(
                      std::stoul(
                          header.substr(
                              content_length_header.size())));
            }
            catch (const std::exception &)
            {
              content_length = 0;
            }
          }
        }

        std::string request_body(
            (std::istreambuf_iterator<char>(input)),
            {});

        if (request_body.size() <
            content_length)
        {
          asio::read(
              socket,
              request,
              asio::transfer_exactly(
                  content_length -
                  request_body.size()),
              error);

          if (error)
          {
            continue;
          }

          input.clear();

          request_body.append(
              (std::istreambuf_iterator<char>(input)),
              {});
        }

        int status = 200;
        std::string body;

        const char *type =
            "application/json";

        if (method.empty() ||
            path.empty() ||
            version.rfind("HTTP/", 0) != 0)
        {
          status = 400;
          body =
              "{\"error\":\"malformed request\"}";
        }
        else if (path == "/")
        {
          body =
              softadastra::webui_assets::index_html;

          type =
              "text/html; charset=utf-8";
        }
        else if (path == "/app.js")
        {
          body =
              softadastra::webui_assets::app_js;

          type =
              "application/javascript; charset=utf-8";
        }
        else if (path == "/style.css")
        {
          body =
              softadastra::webui_assets::style_css;

          type =
              "text/css; charset=utf-8";
        }
        else if (
            path == "/api/project-folder" &&
            method == "POST")
        {
          const auto selected =
              directory_chooser_();

          if (selected.status ==
              DirectoryChooserStatus::Cancelled)
          {
            body =
                "{\"cancelled\":true}";
          }
          else if (
              selected.status ==
              DirectoryChooserStatus::Unavailable)
          {
            status = 503;
            body =
                "{\"error\":\"A folder picker is not available on this computer.\"}";
          }
          else
          {
            std::string config_error;

            const auto config =
                ProjectConfigFile::find(
                    selected.path,
                    &config_error);

            if (!config_error.empty())
            {
              status = 400;
              body =
                  "{\"error\":\"The selected project has an invalid softadastra.toml file.\"}";
            }
            else
            {
              const auto &project_directory =
                  config
                      ? config->first
                      : selected.path;

              const std::string name =
                  config
                      ? config->second.name
                      : project_directory
                            .filename()
                            .string();

              body =
                  "{\"project_directory\":\"" +
                  escape_json(
                      project_directory.string()) +
                  "\",\"name\":\"" +
                  escape_json(name) +
                  "\",\"configured\":" +
                  (config
                       ? "true"
                       : "false");

              if (config)
              {
                body +=
                    ",\"command\":\"" +
                    escape_json(
                        config->second.command) +
                    "\"";

                body +=
                    ",\"access_points\":";

                auto points =
                    config->second.access_points;

                if (points.empty() &&
                    config->second.access)
                {
                  points.push_back(
                      *config->second.access);
                }

                SoftwareEntry configured(
                    SoftwareId("project"),
                    ProcessSpec("project"),
                    std::nullopt,
                    std::move(points));

                body +=
                    access_json(
                        configured,
                        nullptr);
              }

              body += "}";
            }
          }
        }
        else if (path == "/api/host")
        {
          const auto observation =
              observe_host(
                  client_,
                  NativeDataDirectory::path());

          status =
              observation.available()
                  ? 200
                  : 503;

          if (status == 200)
          {
            const auto access =
                client_.local_access();

            body =
                "{\"status\":\"running\",\"hostname\":\"" +
                escape_json(
                    access
                        ? access->host_name
                        : "") +
                "\",\"local_ip\":\"" +
                escape_json(
                    access
                        ? access->primary_ipv4
                        : "") +
                "\",\"connectivity\":\"" +
                (client_.connectivity_available()
                     ? "available"
                     : "unavailable") +
                "\",\"remote_status\":\"" +
                (client_.connected()
                     ? "connected"
                     : "disconnected") +
                "\"}";
          }
          else
          {
            body =
                std::string("{\"status\":\"") +
                host_availability_name(observation.state) +
                "\"}";
          }
        }
        else if (path == "/api/software")
        {
          if (method == "POST")
          {
            const auto name =
                json_string(
                    request_body,
                    "name");

            const auto directory =
                json_string(
                    request_body,
                    "project_directory");

            const auto command =
                json_string(
                    request_body,
                    "command");

            auto access_points =
                json_access_points(
                    request_body);

            // Keep requests produced by earlier Web UI builds working.
            if (access_points &&
                access_points->empty())
            {
              const auto protocol =
                  json_string(
                      request_body,
                      "protocol");

              const auto port_text =
                  json_string(
                      request_body,
                      "port");

              if (protocol ||
                  port_text)
              {
                const auto legacy =
                    json_access_points(
                        "{\"access_points\":[{\"protocol\":\"" +
                        protocol.value_or("") +
                        "\",\"port\":\"" +
                        port_text.value_or("") +
                        "\"}]}");

                access_points =
                    legacy;
              }
            }

            if (!name ||
                name->empty() ||
                !directory ||
                directory->empty() ||
                !command ||
                command->empty())
            {
              status = 400;
              body =
                  "{\"error\":\"Enter a name, project directory, and command.\"}";
            }
            else if (
                !std::filesystem::is_directory(
                    *directory))
            {
              status = 400;
              body =
                  "{\"error\":\"The project directory does not exist.\"}";
            }
            else if (!access_points)
            {
              status = 400;
              body =
                  "{\"error\":\"Each access endpoint needs HTTP or WebSocket and a port between 1 and 65535.\"}";
            }
            else
            {
              bool duplicate = false;

              for (const auto &entry :
                   client_.software())
              {
                if (entry.name() == *name)
                {
                  duplicate = true;
                  break;
                }
              }

              if (status == 200 &&
                  duplicate)
              {
                status = 400;
                body =
                    "{\"error\":\"An application with this name already exists.\"}";
              }

              if (status == 200)
              {
                const SoftwareId id(
                    SoftwareId::generate());

#if defined(_WIN32)

                const ProcessSpec process(
                    "cmd.exe",
                    {"/C", *command},
                    *directory);

#else

                const ProcessSpec process(
                    "/bin/sh",
                    {"-lc", *command},
                    *directory);

#endif

                const bool registered =
                    client_.register_software(
                        id,
                        process,
                        std::nullopt,
                        std::nullopt,
                        *name);

                const bool synchronized =
                    registered &&
                    (access_points->empty() ||
                     client_.synchronize_software(
                         id,
                         process,
                         std::move(*access_points),
                         *name));

                if (registered &&
                    !synchronized)
                {
                  static_cast<void>(
                      client_.remove_software(id));
                }

                status =
                    synchronized
                        ? 200
                        : 503;

                body =
                    synchronized
                        ? "{\"ok\":true}"
                        : "{\"error\":\"Softadastra could not add this application.\"}";
              }
            }
          }
          else if (method != "GET")
          {
            status = 404;
            body =
                "{\"error\":\"not found\"}";
          }
          else
          {
            body = "[";
            bool first = true;

            for (const auto &entry :
                 client_.software())
            {
              if (!first)
              {
                body += ",";
              }

              first = false;

              const auto accesses = client_.local_accesses(entry.id());

              body +=
                  "{\"name\":\"" +
                  escape_json(entry.name()) +
                  "\",\"state\":\"" +
                  softadastra::software_state_name(entry.state()) +
                  "\",\"project_directory\":\"" +
                  escape_json(
                      entry.process_spec()
                          .working_directory()
                          .value_or("")) +
                  "\",\"command\":\"" +
                  escape_json(
                      command_name(entry)) +
                  "\",\"accesses\":" +
                  access_json(
                      entry,
                      accesses ? &*accesses : nullptr) +
                  ",\"access_configured\":\"";

              if (const auto access =
                      entry.access_point())
              {
                body +=
                    std::string(
                        AccessPoint::name(
                            access->protocol())) +
                    ":" +
                    std::to_string(
                        access->port());
              }

              body += "\"}";
            }

            body += "]";
          }
        }
        else if (
            path.rfind(
                "/api/software/",
                0) == 0)
        {
          const auto tail =
              path.substr(14);

          const auto slash =
              tail.find('/');

          const auto encoded_name =
              tail.substr(
                  0,
                  slash);

          const auto name =
              url_decode(
                  encoded_name);

          if (!name)
          {
            status = 400;
            body =
                "{\"error\":\"invalid application name\"}";
          }

          std::optional<SoftwareEntry> found;

          if (name)
          {
            for (const auto &entry :
                 client_.software())
            {
              if (entry.name() == *name)
              {
                found = entry;
                break;
              }
            }
          }

          if (status == 400)
          {
          }
          else if (!found)
          {
            status = 404;
            body =
                "{\"error\":\"application not found\"}";
          }
          else if (
              method == "PUT" &&
              slash == std::string::npos)
          {
            const auto updated_name =
                json_string(
                    request_body,
                    "name");

            const auto directory =
                json_string(
                    request_body,
                    "project_directory");

            const auto command =
                json_string(
                    request_body,
                    "command");

            const auto points =
                json_access_points(
                    request_body);

            if (found->state() ==
                SoftwareState::Running)
            {
              status = 400;
              body =
                  "{\"error\":\"Stop the application before editing its configuration.\"}";
            }
            else if (
                !updated_name ||
                updated_name->empty() ||
                !directory ||
                directory->empty() ||
                !command ||
                command->empty())
            {
              status = 400;
              body =
                  "{\"error\":\"Enter a name, project directory, and command.\"}";
            }
            else if (
                !std::filesystem::is_directory(
                    *directory))
            {
              status = 400;
              body =
                  "{\"error\":\"The project directory does not exist.\"}";
            }
            else if (!points)
            {
              status = 400;
              body =
                  "{\"error\":\"Each access endpoint needs HTTP or WebSocket and a port between 1 and 65535.\"}";
            }
            else
            {
#if defined(_WIN32)

              const ProcessSpec process(
                  "cmd.exe",
                  {"/C", *command},
                  *directory);

#else

              const ProcessSpec process(
                  "/bin/sh",
                  {"-lc", *command},
                  *directory);

#endif

              const bool saved =
                  client_.synchronize_software(
                      found->id(),
                      process,
                      *points,
                      *updated_name);

              status =
                  saved
                      ? 200
                      : 503;

              body =
                  saved
                      ? "{\"ok\":true}"
                      : "{\"error\":\"Softadastra could not save this configuration.\"}";
            }
          }
          else if (
              slash != std::string::npos &&
              tail.substr(slash + 1) ==
                  "access")
          {
            const auto access =
                client_.local_access(
                    found->id());

            body =
                access
                    ? "{\"url\":\"" +
                          escape_json(access->url) +
                          "\",\"ipv4\":\"" +
                          escape_json(access->ipv4) +
                          "\"}"
                    : "{}";
          }
          else if (
              slash != std::string::npos &&
              tail.substr(slash + 1) ==
                  "logs" &&
              method == "GET")
          {
            const auto requested_offset =
                query.empty()
                    ? std::optional<std::uintmax_t>{}
                    : query_offset(query);

            if (!query.empty() &&
                !requested_offset)
            {
              status = 400;
              body =
                  "{\"error\":\"invalid log offset\"}";
            }
            else
            {
              const auto logs =
                  client_.logs_since(
                      found->id(),
                      requested_offset);

              const bool running =
                  found->state() ==
                  SoftwareState::Running;

              body =
                  logs
                      ? "{\"logs\":\"" +
                            escape_json(logs->logs) +
                            "\",\"offset\":" +
                            std::to_string(logs->offset) +
                            ",\"reset\":" +
                            (logs->reset
                                 ? "true"
                                 : "false") +
                            ",\"running\":" +
                            (running
                                 ? "true"
                                 : "false") +
                            "}"
                      : "{\"error\":\"logs unavailable\"}";

              status =
                  logs
                      ? 200
                      : 503;
            }
          }
          else if (
              slash != std::string::npos &&
              tail.substr(slash + 1) ==
                  "logs/clear" &&
              method == "POST")
          {
            const bool ok =
                client_.clear_logs(
                    found->id());

            body =
                ok
                    ? "{\"ok\":true}"
                    : "{\"error\":\"failed to clear logs\"}";

            status =
                ok
                    ? 200
                    : 503;
          }
          else if (
              method == "DELETE" &&
              slash == std::string::npos)
          {
            const bool ok =
                client_.remove_software(
                    found->id());

            body =
                ok
                    ? "{\"ok\":true}"
                    : "{\"error\":\"failed to remove application\"}";

            status =
                ok
                    ? 200
                    : 503;
          }
          else if (
              slash != std::string::npos &&
              method == "POST")
          {
            const auto action =
                tail.substr(
                    slash + 1);

            if (action == "remove")
            {
              const bool ok =
                  client_.remove_software(
                      found->id());

              body =
                  ok
                      ? "{\"ok\":true}"
                      : "{\"error\":\"failed to remove application\"}";

              status =
                  ok
                      ? 200
                      : 503;
            }
            else
            {
              const auto result =
                  action == "start"
                      ? client_.start_software(
                            found->id())
                  : action == "stop"
                      ? client_.stop_software(
                            found->id())
                  : action == "restart"
                      ? client_.restart_software(
                            found->id())
                      : SoftwareOperationResult(
                            SoftwareOperationError::LaunchFailed);

              const bool ok =
                  result.succeeded();

              body =
                  ok
                      ? "{\"ok\":true}"
                      : "{\"error\":\"application operation failed\"}";

              status =
                  ok
                      ? 200
                      : 503;
            }
          }
          else
          {
            status = 404;
            body =
                "{\"error\":\"not found\"}";
          }
        }
        else
        {
          status = 404;
          body =
              "{\"error\":\"not found\"}";
        }

        static_cast<void>(
            asio::write(
                socket,
                asio::buffer(
                    response(
                        status,
                        body,
                        type)),
                error));
      }
    }
    catch (...)
    {
      publish_startup(
          false,
          0);
    }
  }

} // namespace softadastra
