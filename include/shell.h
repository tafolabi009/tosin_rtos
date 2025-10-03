#ifndef SHELL_H
#define SHELL_H

#include "types.h"

/* Shell command handler type */
typedef int32_t (*shell_cmd_handler_t)(int argc, char **argv);

/* Shell command structure */
typedef struct {
    const char *name;
    const char *help;
    shell_cmd_handler_t handler;
} shell_cmd_t;

/* Shell functions */
void shell_init(void);
void shell_run(void);
int32_t shell_register_command(const char *name, const char *help, 
                               shell_cmd_handler_t handler);

#endif /* SHELL_H */
