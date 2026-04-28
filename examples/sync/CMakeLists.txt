function(softadastra_sync_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target}
        PRIVATE
            softadastra::core
            softadastra::fs
            softadastra::wal
            softadastra::store
            softadastra::sync
    )
endfunction()

softadastra_sync_add_example(
    softadastra_sync_example_basic
    "${CMAKE_CURRENT_SOURCE_DIR}/basic_sync.cpp"
)

softadastra_sync_add_example(
    softadastra_sync_example_remote_apply
    "${CMAKE_CURRENT_SOURCE_DIR}/remote_apply.cpp"
)

softadastra_sync_add_example(
    softadastra_sync_example_retry
    "${CMAKE_CURRENT_SOURCE_DIR}/retry.cpp"
)
