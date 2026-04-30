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

softadastra_sync_add_example(
    softadastra_sync_example_ack_tracker
    "${CMAKE_CURRENT_SOURCE_DIR}/ack_tracker.cpp"
)

softadastra_sync_add_example(
    softadastra_sync_example_conflict_policy
    "${CMAKE_CURRENT_SOURCE_DIR}/conflict_policy.cpp"
)

softadastra_sync_add_example(
    softadastra_sync_example_manual_queue
    "${CMAKE_CURRENT_SOURCE_DIR}/manual_queue.cpp"
)

softadastra_sync_add_example(
    softadastra_sync_example_scheduler_tick
    "${CMAKE_CURRENT_SOURCE_DIR}/scheduler_tick.cpp"
)
