function(softadastra_fs_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target}
        PRIVATE
            softadastra::core
            softadastra::fs
    )
endfunction()

softadastra_fs_add_example(
    softadastra_fs_example_diff
    "${CMAKE_CURRENT_SOURCE_DIR}/diff.cpp"
)

softadastra_fs_add_example(
    softadastra_fs_example_scan
    "${CMAKE_CURRENT_SOURCE_DIR}/scan.cpp"
)

softadastra_fs_add_example(
    softadastra_fs_example_snapshot
    "${CMAKE_CURRENT_SOURCE_DIR}/snapshot.cpp"
)

softadastra_fs_add_example(
    softadastra_fs_example_watch
    "${CMAKE_CURRENT_SOURCE_DIR}/watch.cpp"
)
