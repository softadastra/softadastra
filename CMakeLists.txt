cmake_minimum_required(VERSION 3.21)

project(
  softadastra
  LANGUAGES CXX
)

# Keep the Web UI maintainable as normal source files while embedding it in the
# offline executable.  Reconfigure when an asset changes so the generated
# header always follows the source assets.
set(SOFTADASTRA_WEBUI_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/webui/assets")
set(SOFTADASTRA_WEBUI_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/webui")
set(SOFTADASTRA_WEBUI_ASSETS
  "${SOFTADASTRA_WEBUI_ASSET_DIR}/index.html"
  "${SOFTADASTRA_WEBUI_ASSET_DIR}/app.js"
  "${SOFTADASTRA_WEBUI_ASSET_DIR}/style.css"
)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${SOFTADASTRA_WEBUI_ASSETS})
file(MAKE_DIRECTORY "${SOFTADASTRA_WEBUI_GENERATED_DIR}")
file(READ "${SOFTADASTRA_WEBUI_ASSET_DIR}/index.html" SOFTADASTRA_WEBUI_INDEX_HTML)
file(READ "${SOFTADASTRA_WEBUI_ASSET_DIR}/app.js" SOFTADASTRA_WEBUI_APP_JS)
file(READ "${SOFTADASTRA_WEBUI_ASSET_DIR}/style.css" SOFTADASTRA_WEBUI_STYLE_CSS)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/webui/WebUiAssets.hpp.in"
  "${SOFTADASTRA_WEBUI_GENERATED_DIR}/WebUiAssets.hpp"
  @ONLY
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(
  SOFTADASTRA_BUILD_TESTS
  "Build Softadastra tests"
  ON
)

find_package(Vix CONFIG QUIET)
if(NOT Vix_FOUND)
  include(FetchContent)
  set(VIX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(VIX_SDK_PROFILE default CACHE STRING "" FORCE)
  set(VIX_ENABLE_NOTE OFF CACHE BOOL "" FORCE)
  set(VIX_ENABLE_TESTS_MODULE ON CACHE BOOL "" FORCE)
  FetchContent_Declare(
    vix
    GIT_REPOSITORY https://github.com/vixcpp/vix.git
    GIT_TAG v2.7.8
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(vix)
endif()
find_package(OpenSSL REQUIRED)

add_library(
  softadastra_internal
  STATIC
  src/cli/Cli.cpp
  src/cli/AccessUrl.cpp
  src/connectivity/ConnectivityManager.cpp
  src/control/ControlClient.cpp
  src/control/ControlServer.cpp
  src/control/LocalControlProtocol.cpp
  src/control/LocalControlServer.cpp
  src/control/RemoteAccessConfig.cpp
  src/webui/WebUiServer.cpp
  src/gateway/ControlLocalGatewayTargetResolver.cpp
  src/host/Host.cpp
  src/host/HostIdentity.cpp
  src/host/HostProfile.cpp
  src/host/HostPeerIdentity.cpp
  src/host/HostPeerClient.cpp
  src/host/HostPeerServer.cpp
  src/host/HostPeerTrust.cpp
  src/host/HostLoop.cpp
  src/host/LocalAccess.cpp
  src/host/HttpProxy.cpp
  src/host/RemoteReachability.cpp
  src/host/HostService.cpp
  src/host/HostState.cpp
  src/host/HostStateFile.cpp
  src/platform/NativeNetwork.cpp
  src/platform/ManagedNetwork.cpp
  src/platform/NativeManagedNetwork.cpp
  src/platform/GatewayCapabilityProvisioning.cpp
  src/platform/LocalDnsDelegation.cpp
  src/platform/NativeLocalDnsDelegation.cpp
  src/platform/NativeLocalGatewaySocketProvider.cpp
  src/platform/NativeDataDirectory.cpp
  src/platform/NativeDirectoryChooser.cpp
  src/platform/HostInstanceLock.cpp
  src/platform/MdnsPublisher.cpp
  src/platform/NativePlatform.cpp
  src/platform/QrCode.cpp
  src/platform/NativeProcess.cpp
  src/platform/NativeProcessLauncher.cpp
  src/platform/NativeService.cpp
  src/software/SoftwareManager.cpp
  src/software/ProjectIdentity.cpp
  src/software/ProjectConfig.cpp
  src/software/SoftwareRegistrationFormat.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(
    softadastra_internal
    PRIVATE
      src/host/LocalDns.cpp
      src/host/LocalGateway.cpp
      src/host/LocalGatewayProcessEndpoint.cpp
  )
endif()
target_sources(softadastra_internal PRIVATE src/host/LocalReachability.cpp)

target_include_directories(
  softadastra_internal
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PRIVATE
    ${SOFTADASTRA_WEBUI_GENERATED_DIR}
)

target_link_libraries(
  softadastra_internal
  PUBLIC
    vix::vix
    OpenSSL::Crypto
    OpenSSL::SSL
)

if(WIN32)
  target_link_libraries(
    softadastra_internal
    PRIVATE
      advapi32
      iphlpapi
      ole32
      shell32
      ws2_32
  )
endif()

if(MSVC)
  target_compile_options(
    softadastra_internal
    PRIVATE
      /W4
      /permissive-
  )
else()
  target_compile_options(
    softadastra_internal
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
  )
endif()

add_executable(
  softadastra
  src/main.cpp
)

target_link_libraries(
  softadastra
  PRIVATE
    softadastra_internal
)

if(MSVC)
  target_compile_options(
    softadastra
    PRIVATE
      /W4
      /permissive-
  )
else()
  target_compile_options(
    softadastra
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
  )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(
    softadastra-gateway
    src/gateway/main.cpp
  )

  target_link_libraries(
    softadastra-gateway
    PRIVATE
      softadastra_internal
  )

  if(MSVC)
    target_compile_options(softadastra-gateway PRIVATE /W4 /permissive-)
  else()
    target_compile_options(softadastra-gateway PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
  endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(
    TARGETS softadastra
    RUNTIME DESTINATION bin
  )
  install(
    TARGETS softadastra-gateway
    RUNTIME DESTINATION bin
  )
  install(
    FILES packaging/softadastra.service
    DESTINATION lib/systemd/system
  )
endif()

if(WIN32)
  install(
    TARGETS softadastra
    RUNTIME_DEPENDENCIES
      PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-"
      POST_EXCLUDE_REGEXES ".*/[Ss]ystem32/.*"
    RUNTIME DESTINATION bin
  )
endif()

set(CPACK_PACKAGE_NAME "Softadastra")
set(CPACK_PACKAGE_VENDOR "Softadastra")
set(CPACK_PACKAGE_CONTACT "Softadastra <support@softadastra.com>")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Softadastra")
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd")
endif()
include(CPack)

if(SOFTADASTRA_BUILD_TESTS)
  include(CTest)

  add_test(
    NAME softadastra_box_install_test
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/box/install_test.sh
  )

  set_tests_properties(
    softadastra_box_install_test
    PROPERTIES
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  find_package(GTest CONFIG QUIET)

  if(NOT GTest_FOUND)
    include(FetchContent)

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    )

    if(WIN32)
      set(
        gtest_force_shared_crt
        ON
        CACHE BOOL ""
        FORCE
      )
    endif()

    FetchContent_MakeAvailable(googletest)
  endif()

  include(GoogleTest)

  function(softadastra_add_test target source)
    add_executable(
      ${target}
      ${source}
    )

    target_link_libraries(
      ${target}
      PRIVATE
        softadastra_internal
        GTest::gtest_main
    )

    if(MSVC)
      target_compile_options(
        ${target}
        PRIVATE
          /W4
          /permissive-
      )
    else()
      target_compile_options(
        ${target}
        PRIVATE
          -Wall
          -Wextra
          -Wpedantic
          -Wconversion
          -Wsign-conversion
      )
    endif()

    if(target STREQUAL "softadastra_local_control_test")
      gtest_discover_tests(${target} PROPERTIES RUN_SERIAL TRUE)
    else()
      gtest_discover_tests(${target})
    endif()
  endfunction()

  softadastra_add_test(
    softadastra_host_state_file_test
    tests/host/host_state_file_test.cpp
  )

  softadastra_add_test(
    softadastra_host_profile_test
    tests/host/host_profile_test.cpp
  )

  softadastra_add_test(
    softadastra_local_access_test
    tests/host/local_access_test.cpp
  )

  softadastra_add_test(
    softadastra_web_ui_server_test
    tests/webui/web_ui_server_test.cpp
  )

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    softadastra_add_test(softadastra_local_gateway_test tests/host/local_gateway_test.cpp)
    softadastra_add_test(softadastra_local_gateway_process_endpoint_test tests/host/local_gateway_process_endpoint_test.cpp)
    softadastra_add_test(softadastra_local_dns_test tests/host/local_dns_test.cpp)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    softadastra_add_test(
      softadastra_local_dns_delegation_test
      tests/platform/local_dns_delegation_test.cpp
    )
    softadastra_add_test(softadastra_local_reachability_test tests/host/local_reachability_test.cpp)
    softadastra_add_test(
      softadastra_local_gateway_socket_provider_test
      tests/platform/local_gateway_socket_provider_test.cpp
    )
    softadastra_add_test(
      softadastra_gateway_capability_provisioning_test
      tests/platform/gateway_capability_provisioning_test.cpp
    )
    softadastra_add_test(
      softadastra_native_managed_network_test
      tests/platform/native_managed_network_test.cpp
    )
  endif()

  softadastra_add_test(
    softadastra_remote_reachability_test
    tests/host/remote_reachability_test.cpp
  )

  softadastra_add_test(
    softadastra_software_registration_format_test
    tests/software/software_registration_format_test.cpp
  )

  softadastra_add_test(
    softadastra_native_data_directory_test
    tests/platform/native_data_directory_test.cpp
  )

  softadastra_add_test(
    softadastra_software_id_test
    tests/software/software_id_test.cpp
  )

  softadastra_add_test(
    softadastra_software_state_test
    tests/software/software_state_test.cpp
  )

  softadastra_add_test(
    softadastra_software_entry_test
    tests/software/software_entry_test.cpp
  )

  softadastra_add_test(
    softadastra_access_point_test
    tests/software/access_point_test.cpp
  )

  softadastra_add_test(
    softadastra_local_name_test
    tests/software/local_name_test.cpp
  )

  softadastra_add_test(
    softadastra_project_identity_test
    tests/software/project_identity_test.cpp
  )

  softadastra_add_test(
    softadastra_software_manager_test
    tests/software/software_manager_test.cpp
  )

  softadastra_add_test(
    softadastra_process_spec_test
    tests/platform/process_spec_test.cpp
  )

  softadastra_add_test(
    softadastra_process_test
    tests/platform/process_test.cpp
  )

  softadastra_add_test(
    softadastra_process_launcher_test
    tests/platform/process_launcher_test.cpp
  )

  softadastra_add_test(
    softadastra_service_test
    tests/platform/service_test.cpp
  )

  softadastra_add_test(
    softadastra_host_instance_lock_test
    tests/platform/host_instance_lock_test.cpp
  )

  softadastra_add_test(
    softadastra_access_url_test
    tests/cli/access_url_test.cpp
  )

  softadastra_add_test(
    softadastra_cli_test
    tests/cli/cli_test.cpp
  )

  softadastra_add_test(
    softadastra_network_test
    tests/platform/network_test.cpp
  )

  softadastra_add_test(
    softadastra_platform_test
    tests/platform/platform_test.cpp
  )

  softadastra_add_test(
    softadastra_native_process_test
    tests/platform/native_process_test.cpp
  )

  softadastra_add_test(
    softadastra_native_process_launcher_test
    tests/platform/native_process_launcher_test.cpp
  )

  softadastra_add_test(
    softadastra_native_network_test
    tests/platform/native_network_test.cpp
  )

  softadastra_add_test(
    softadastra_mdns_publisher_test
    tests/platform/mdns_publisher_test.cpp
  )

  softadastra_add_test(
    softadastra_native_service_test
    tests/platform/native_service_test.cpp
  )

  softadastra_add_test(
    softadastra_native_platform_test
    tests/platform/native_platform_test.cpp
  )

  softadastra_add_test(
    softadastra_host_state_test
    tests/host/host_state_test.cpp
  )

  softadastra_add_test(
    softadastra_host_test
    tests/host/host_test.cpp
  )

  softadastra_add_test(
    softadastra_host_identity_test
    tests/host/host_identity_test.cpp
  )

  softadastra_add_test(
    softadastra_host_peer_identity_test
    tests/host/host_peer_identity_test.cpp
  )

  softadastra_add_test(
    softadastra_host_peer_trust_test
    tests/host/host_peer_trust_test.cpp
  )

  softadastra_add_test(
    softadastra_host_peer_transport_test
    tests/host/host_peer_transport_test.cpp
  )

  softadastra_add_test(
    softadastra_remote_access_config_test
    tests/control/remote_access_config_test.cpp
  )


  softadastra_add_test(
    softadastra_host_loop_test
    tests/host/host_loop_test.cpp
  )

  softadastra_add_test(
    softadastra_host_service_test
    tests/host/host_service_test.cpp
  )

  softadastra_add_test(
    softadastra_connectivity_manager_test
    tests/connectivity/connectivity_manager_test.cpp
  )

  softadastra_add_test(
    softadastra_control_server_test
    tests/control/control_server_test.cpp
  )

  softadastra_add_test(
    softadastra_control_client_test
    tests/control/control_client_test.cpp
  )

  softadastra_add_test(
    softadastra_local_control_protocol_test
    tests/control/local_control_protocol_test.cpp
  )

  softadastra_add_test(
    softadastra_control_local_gateway_target_resolver_test
    tests/gateway/control_local_gateway_target_resolver_test.cpp
  )

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    softadastra_add_test(softadastra_gateway_process_test tests/gateway/gateway_process_test.cpp)
    add_dependencies(softadastra_gateway_process_test softadastra-gateway)
    target_compile_definitions(softadastra_gateway_process_test PRIVATE SOFTADASTRA_GATEWAY_EXECUTABLE="$<TARGET_FILE:softadastra-gateway>")
  endif()

  softadastra_add_test(
    softadastra_host_runtime_test
    tests/integration/host_runtime_test.cpp
  )

  softadastra_add_test(
    softadastra_local_control_test
    tests/integration/local_control_test.cpp
  )

  target_compile_definitions(
    softadastra_local_control_test
    PRIVATE
      SOFTADASTRA_EXECUTABLE="$<TARGET_FILE:softadastra>"
  )
endif()
