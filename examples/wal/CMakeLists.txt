function(softadastra_wal_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target}
        PRIVATE
            softadastra::core
            softadastra::fs
            softadastra::wal
    )
endfunction()

softadastra_wal_add_example(
    softadastra_wal_example_event
    "${CMAKE_CURRENT_SOURCE_DIR}/wal_event.cpp"
)

softadastra_wal_add_example(
    softadastra_wal_example_read
    "${CMAKE_CURRENT_SOURCE_DIR}/wal_read.cpp"
)

softadastra_wal_add_example(
    softadastra_wal_example_replay_to_store
    "${CMAKE_CURRENT_SOURCE_DIR}/wal_replay_to_store.cpp"
)

softadastra_wal_add_example(
    softadastra_wal_example_stream
    "${CMAKE_CURRENT_SOURCE_DIR}/wal_stream.cpp"
)

softadastra_wal_add_example(
    softadastra_wal_example_write
    "${CMAKE_CURRENT_SOURCE_DIR}/wal_write.cpp"
)
