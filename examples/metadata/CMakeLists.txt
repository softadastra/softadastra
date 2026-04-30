function(softadastra_metadata_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target}
        PRIVATE
            softadastra::core
            softadastra::fs
            softadastra::wal
            softadastra::store
            softadastra::sync
            softadastra::transport
            softadastra::discovery
            softadastra::metadata
    )
endfunction()

softadastra_metadata_add_example(
    softadastra_metadata_example_encode_decode_demo
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_encode_decode_demo.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_local_snapshot
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_local_snapshot.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_minimal
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_minimal.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_registry_demo
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_registry_demo.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_capabilities
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_capabilities.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_custom_provider
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_custom_provider.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_registry_filter
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_registry_filter.cpp"
)

softadastra_metadata_add_example(
    softadastra_metadata_example_service_demo
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata_service_demo.cpp"
)
