#include "protocol-task.h"

#include <stdio.h>
#include <string.h>

static api_t* api = NULL;
static int commands_count = 0;

static void protocol_print_help(void)
{
    for (int i = 0; i < commands_count; i++)
    {
        printf("Команда '%s': '%s'\n",
               api[i].command_name,
               api[i].command_help);
    }

    printf("Команда 'help': 'print commands description'\n");
}

void protocol_task_init(api_t* device_api)
{
    api = device_api;
    commands_count = 0;

    while (api[commands_count].command_name != NULL)
    {
        commands_count++;
    }
}

void protocol_task_handle(char* command_string)
{
    if (command_string == NULL)
    {
        return;
    }

    const char* command_name = command_string;
    const char* command_args = "";

    char* space_symbol = strchr(command_string, ' ');

    if (space_symbol != NULL)
    {
        *space_symbol = '\0';
        command_args = space_symbol + 1;
    }

    printf("received command: '%s' with args: '%s'\n", command_name, command_args);

    if (strcmp(command_name, "help") == 0)
    {
        protocol_print_help();
        return;
    }

    for (int i = 0; i < commands_count; i++)
    {
        if (strcmp(command_name, api[i].command_name) == 0)
        {
            api[i].command_callback(command_args);
            return;
        }
    }

    printf("unknown command: %s\n", command_name);
}