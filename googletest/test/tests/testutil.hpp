#include <stdio.h>

// testutil_cmd.cpp
char* testutil_exec_cmd_and_read_output(const char* cmd, size_t* stdout_size);
void testutil_exec_cmd(const char *cmd);

// testutil_file.cpp
char* testutil_read_file_to_buffer(const char* filepath, size_t* file_size);
char** testutil_read_file_to_lines(const char* filepath, size_t* line_count);
void testutil_free_lines(char** lines, size_t line_count);

// testutil_fixture.cpp
char* testutil_read_fixture_to_buffer(const char* filepath, size_t* file_size);
char** testutil_read_fixture_to_lines(const char* filepath, size_t* line_count);
