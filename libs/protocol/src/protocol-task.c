#include "protocol-task.h"

#include <stdio.h>
#include <string.h>

static api_t* api = NULL;

void protocol_task_init(api_t* device_api)
{
    api = device_api;
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

    for (int i = 0; api[i].command_name != NULL; i++)
    {
        if (strcmp(command_name, api[i].command_name) == 0)
        {
            api[i].command_callback(command_args);
            return;
        }
    }

    printf("unknown command: %s\n", command_name);
}