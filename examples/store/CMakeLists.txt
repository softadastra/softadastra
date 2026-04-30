function(softadastra_store_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target}
        PRIVATE
            softadastra::core
            softadastra::fs
            softadastra::wal
            softadastra::store
    )
endfunction()

softadastra_store_add_example(
    softadastra_store_example_basic
    "${CMAKE_CURRENT_SOURCE_DIR}/basic_store.cpp"
)

softadastra_store_add_example(
    softadastra_store_example_recovery
    "${CMAKE_CURRENT_SOURCE_DIR}/recovery_demo.cpp"
)

softadastra_store_add_example(
    softadastra_store_example_wal_usage
    "${CMAKE_CURRENT_SOURCE_DIR}/wal_usage.cpp"
)

softadastra_store_add_example(
    softadastra_store_example_memory_only
    "${CMAKE_CURRENT_SOURCE_DIR}/memory_only.cpp"
)

softadastra_store_add_example(
    softadastra_store_example_operation_codec
    "${CMAKE_CURRENT_SOURCE_DIR}/operation_codec.cpp"
)

softadastra_store_add_example(
    softadastra_store_example_snapshot_from_wal
    "${CMAKE_CURRENT_SOURCE_DIR}/snapshot_from_wal.cpp"
)
