# examples/CMakeLists.txt

message(STATUS "[softadastra] configuring umbrella examples")

function(softadastra_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
endfunction()

# ======================================================
# Per-module example folders
# ======================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/CMakeLists.txt")
    add_subdirectory(core)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fs/CMakeLists.txt")
    add_subdirectory(fs)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wal/CMakeLists.txt")
    add_subdirectory(wal)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/store/CMakeLists.txt")
    add_subdirectory(store)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sync/CMakeLists.txt")
    add_subdirectory(sync)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/transport/CMakeLists.txt")
    add_subdirectory(transport)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/discovery/CMakeLists.txt")
    add_subdirectory(discovery)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/metadata/CMakeLists.txt")
    add_subdirectory(metadata)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cli/CMakeLists.txt")
    add_subdirectory(cli)
endif()
