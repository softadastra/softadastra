function(softadastra_transport_add_example target source_file)
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
    )
endfunction()

softadastra_transport_add_example(
    softadastra_transport_example_basic_client
    "${CMAKE_CURRENT_SOURCE_DIR}/basic_client.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_basic_server
    "${CMAKE_CURRENT_SOURCE_DIR}/basic_server.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_drive_end_to_end_client
    "${CMAKE_CURRENT_SOURCE_DIR}/drive_end_to_end_demo_client.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_drive_end_to_end_server
    "${CMAKE_CURRENT_SOURCE_DIR}/drive_end_to_end_demo_server.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_full_sync_client
    "${CMAKE_CURRENT_SOURCE_DIR}/full_sync_demo_client.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_full_sync_server
    "${CMAKE_CURRENT_SOURCE_DIR}/full_sync_demo_server.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_sync_bridge
    "${CMAKE_CURRENT_SOURCE_DIR}/sync_bridge.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_dispatcher_ping
    "${CMAKE_CURRENT_SOURCE_DIR}/dispatcher_ping.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_message_codec
    "${CMAKE_CURRENT_SOURCE_DIR}/message_codec.cpp"
)

softadastra_transport_add_example(
    softadastra_transport_example_peer_registry
    "${CMAKE_CURRENT_SOURCE_DIR}/peer_registry.cpp"
)
