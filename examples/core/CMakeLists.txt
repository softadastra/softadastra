function(softadastra_core_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target} PRIVATE softadastra::core)
endfunction()

softadastra_core_add_example(
    softadastra_core_example_error_result
    "${CMAKE_CURRENT_SOURCE_DIR}/error_result_example.cpp"
)

softadastra_core_add_example(
    softadastra_core_example_ids
    "${CMAKE_CURRENT_SOURCE_DIR}/ids_example.cpp"
)

softadastra_core_add_example(
    softadastra_core_example_time
    "${CMAKE_CURRENT_SOURCE_DIR}/time_example.cpp"
)
