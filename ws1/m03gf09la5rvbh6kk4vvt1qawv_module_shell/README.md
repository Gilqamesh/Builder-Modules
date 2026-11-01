# Interactive module shell

## Goal

Let developers discover and invoke workspace module targets through friendly names, completion, and an interactive command loop. This module owns module-name resolution and command routing; the command framework owns parsing and the Builder CLI owns building and launching selected targets.

See [module_shell.h](module_shell.h) for the public interface.
