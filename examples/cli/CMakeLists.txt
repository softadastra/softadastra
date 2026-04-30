function(softadastra_cli_add_example target source_file)
    add_executable(${target} ${source_file})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target}
        PRIVATE
            softadastra::cli
    )
endfunction()

softadastra_cli_add_example(
    softadastra_cli_example_arg_parser
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_arg_parser.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_commands_demo
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_commands_demo.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_custom_command
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_custom_command.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_minimal
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_minimal.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_registry
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_registry.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_service_single
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_service_single.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_table_formatter
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_table_formatter.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_tokenizer
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_tokenizer.cpp"
)

softadastra_cli_add_example(
    softadastra_cli_example_ui_style
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_ui_style.cpp"
)
