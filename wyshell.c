/*
  Wyshell
  Lily Brongo
  COSC 3750, Spring 2025
  Homework 8
  April 8th, 2025
  This program is a simple shell that can parse and execute commands with
  input/output redirection and background execution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wyscanner.h"

// Structure to store an individual argument in a linked list
typedef struct Argument {
    struct Argument *next;
    char *text;
} Argument;

// Structure to store a single command and its metadata
typedef struct Command {
    struct Command *next;
    char *name;
    Argument *args;
    int has_in, has_out, has_err;
    char *in_file, *out_file, *err_file;
    int background;
} Command;

// Command *create_command(const char *name)
// Parameters: const char *name - the name of the command to create
// Returns: Command* - pointer to the newly created Command struct
// Description: Allocates and initializes a new command node.
Command *create_command(const char *name) {
    Command *cmd = (Command *)malloc(sizeof(Command));
    if (!cmd) return NULL;
    cmd->next = NULL;
    cmd->name = strdup(name);
    cmd->args = NULL;
    cmd->has_in = 0;
    cmd->has_out = 0;
    cmd->has_err = 0;
    cmd->background = 0;
    cmd->in_file = NULL;
    cmd->out_file = NULL;
    cmd->err_file = NULL;
    return cmd;
}

// Argument *create_argument(const char *text)
// Parameters: const char *text - the argument string
// Returns: Argument* - pointer to the newly created Argument struct
// makes a new argument (linked list style)
Argument *create_argument(const char *text) {
    Argument *arg = (Argument *)malloc(sizeof(Argument));
    if (!arg) return NULL;
    arg->text = strdup(text);
    arg->next = NULL;
    return arg;
}

// void append_argument(Command *cmd, const char *text)
// Parameters: Command *cmd, const char *text
// Returns: void
// add arg to the end of the list (so we don’t lose order)
void append_argument(Command *cmd, const char *text) {
    if (!cmd) return;
    Argument *new_arg = create_argument(text);
    if (!new_arg) return;
    if (!cmd->args) {
        cmd->args = new_arg;
    } else {
        Argument *temp = cmd->args;
        while (temp->next) temp = temp->next;
        temp->next = new_arg;
    }
}

// void free_commands(Command *cmd)
// Parameters: Command *cmd - head of the command list
// Returns: void
// cleanup so we don’t leak memory (malloc was used!)
void free_commands(Command *cmd) {
    Command *curr = cmd;
    while (curr) {
        Command *next = curr->next;
        free(curr->name);
        free(curr->in_file);
        free(curr->out_file);
        free(curr->err_file);
        Argument *arg = curr->args;
        while (arg) {
            Argument *next_arg = arg->next;
            free(arg->text);
            free(arg);
            arg = next_arg;
        }
        free(curr);
        curr = next;
    }
}

// int parse_input_line(const char *input, Command **head_ptr)
// Parameters: const char *input - the user input line
//             Command **head_ptr - pointer to store the head of command list
// Returns: int - 0 if parsing succeeded, 1 if an error occurred
// Description: Parses the input line into a linked list of Command structs.
int parse_input_line(const char *input, Command **head_ptr) {
    int token = parse_line(input);
    int error = 0;
    Command *head = NULL;
    Command *current = NULL;

    while (token > EOL && !error) {
        switch (token) {
            case 200:
                if (!current) {
                    current = create_command(lexeme);
                    if (!current) return 1;
                    if (!head) {
                        *head_ptr = head = current;
                    } else {
                        Command *scan = head;
                        while (scan->next) scan = scan->next;
                        scan->next = current;
                    }
                    printf(":--: %s\n", lexeme);
                } else {
                    append_argument(current, lexeme);
                    printf(" --: %s\n", lexeme);
                }
                break;

            case 100: case 102:
                printf(token == 100 ? ">\n" : ">>\n");
                token = parse_line(NULL);
                if (token == 200) {
                    printf(" --: %s\n", lexeme);
                    if (current->has_out) {
                        printf("Ambiguous output redirection\n");
                        error = 1;
                    } else {
                        current->has_out = 1;
                        current->out_file = strdup(lexeme);
                    }
                } else {
                    printf("Error: Expected filename after output redirection\n");
                    error = 1;
                }
                break;

            case 101:
                printf("<\n");
                token = parse_line(NULL);
                if (token == 200) {
                    printf(" --: %s\n", lexeme);
                    if (current->has_in) {
                        printf("Ambiguous input redirection\n");
                        error = 1;
                    } else {
                        current->has_in = 1;
                        current->in_file = strdup(lexeme);
                    }
                } else {
                    printf("Error: Expected filename after input redirection\n");
                    error = 1;
                }
                break;

            case 103: case 104:
                printf(token == 103 ? "2>\n" : "2>>\n");
                token = parse_line(NULL);
                if (token == 200) {
                    printf(" --: %s\n", lexeme);
                    if (current->has_err) {
                        printf("Ambiguous error redirection\n");
                        error = 1;
                    } else {
                        current->has_err = 1;
                        current->err_file = strdup(lexeme);
                    }
                } else {
                    printf("Error: Expected filename after error redirection\n");
                    error = 1;
                }
                break;

            case 105:
                printf("2>&1\n");
                if (current->has_err) {
                    printf("Ambiguous error redirection\n");
                    error = 1;
                } else {
                    current->has_err = 1;
                    if (current->out_file)
                        current->err_file = strdup(current->out_file);
                }
                break;

            case 106:
                printf(";\n");
                current = NULL;
                break;

            case 107:
                printf("|\n");
                current = NULL;
                break;

            case 108:
                printf("&\n");
                {
                    Command *c = head;
                    while (c) {
                        c->background = 1;
                        c = c->next;
                    }
                }
                break;
        }
        token = parse_line(NULL);
    }

    if (token <= EOL) {
        if (token == 96) {
            printf("QUOTE ERROR\n");
        } else if (token == 97) {
            printf("ERROR CHAR: %d\n", error_char);
        } else if (token == 98) {
            perror("system error");
            exit(EXIT_FAILURE);
        }
        error = 1;
    }

    return error;
}

// int main()
// Parameters: none
// Returns: int - status code
// Description: Main loop for reading input, parsing, and cleaning up.
int main() {
    char *line = NULL;
    size_t length = 0;

    while (1) {
        printf("$> ");
        fflush(stdout);

        if (getline(&line, &length, stdin) == -1) {
            perror("getline");
            break;
        }

        Command *cmds = NULL;
        int bad = parse_input_line(line, &cmds);

        if (!bad) {
            printf("--: EOL\n");
        }

        free_commands(cmds);
    }

    free(line);
    return 0;
}
