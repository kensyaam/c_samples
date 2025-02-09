#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 200

typedef struct Config {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    struct Config* next;
} Config;

Config* head = NULL;

void trim_whitespace(char* str) {
    char* end;

    // Leading whitespace
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return;

    // Trailing whitespace
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
}

void add_config(const char* key, const char* value) {
    Config* new_config = (Config*)malloc(sizeof(Config));
    strncpy(new_config->key, key, MAX_KEY_LENGTH);
    strncpy(new_config->value, value, MAX_VALUE_LENGTH);
    new_config->next = head;
    head = new_config;
}

int is_number(const char* str) {
    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

int validate_number(const char* value, int min, int max) {
    if (!is_number(value)) return 0;
    int num = atoi(value);
    return num >= min && num <= max;
}

void parse_config_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Check if the line is too long
        if (strchr(line, '\n') == NULL && !feof(file)) {
            fprintf(stderr, "Line too long, skipping: %s\n", line);
            // Skip the rest of the long line
            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF);
            continue;
        }

        trim_whitespace(line);

        // Ignore comments and empty lines
        if (line[0] == '#' || line[0] == '\0') continue;

        char* separator = strchr(line, '=');
        if (separator == NULL) continue;

        *separator = '\0';
        char* key = line;
        char* value = separator + 1;

        trim_whitespace(key);
        trim_whitespace(value);

        add_config(key, value);
    }

    fclose(file);
}

void print_configs() {
    Config* current = head;
    while (current != NULL) {
        printf("Key: %s, Value: %s\n", current->key, current->value);
        current = current->next;
    }
}

void free_configs() {
    Config* current = head;
    while (current != NULL) {
        Config* next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    const char* config_file = "config.txt";
    parse_config_file(config_file);
    print_configs();
    free_configs();
    return 0;
}