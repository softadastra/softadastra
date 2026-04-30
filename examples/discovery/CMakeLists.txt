function(softadastra_discovery_add_example target source_file)
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
    )
endfunction()

softadastra_discovery_add_example(
    softadastra_discovery_example_announcer
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_announcer.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_listener
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_listener.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_minimal
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_minimal.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_roundtrip_demo
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_roundtrip_demo.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_codec
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_codec.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_engine_setup
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_engine_setup.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_registry
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_registry.cpp"
)

softadastra_discovery_add_example(
    softadastra_discovery_example_service
    "${CMAKE_CURRENT_SOURCE_DIR}/discovery_service.cpp"
)
