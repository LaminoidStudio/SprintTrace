# SprintTrace
C99-library to interact with the plugin interface of ABACOM SprintLayout 6.0.

## Description
SprintTrace is an application library that allows developers to easily build plugins for SprintLayout that can act on whole PCBs or selections.
These plugins can export/import to and from other formats, procedurally generate PCBs, autoroute, vectorize, scale, transform and do much more.

## Supported OSes
Plugins using SprintTrace can either run standalone or be run by a SprintLayout host instance.
If run standalone, SprintTrace plugins run on Windows 7+, Linux and macOS. In hosted mode, plugins will only run on Windows.

## Usage
### As a user
As a user you can place the plugins in any folder and define the 
define the active plugin via 'Tools -> Define plugin'.
Depending on the plugin type (e.g. plugin to export to KiCAD, plugin to 
vectorizing bitmaps, ...) you now want to make an optional selection of 
elements that you want to edit (without selection all elements will be passed to the plugin).
The plugin can now either process the elements directly or display a 
GUI, where further settings can be made (select image to be vectorized, output 
select image to vectorize, select output file, scale factor, 
...).
Finally the plugin can optionally write changes to the elements back to the board. 
into the board.

### As a developer
As a developer you can use CMake to integrate the library into your own 
plugin projects and link it statically.
The plugins are basically built like this:

```
#include <SprintTrace/plugin.h>
#include <SprintTrace/pcb.h>
#include <SprintTrace/elements.h>
#include <SprintTrace/primitives.h>
#include <SprintTrace/errors.h>

int main(int argc, const char* argv[])
{
  // Begin communication with Sprint-Layout, fetch and parse data.
  sprint_require(sprint_plugin_begin(argc, argv));
  // Get the PCB instance
  sprint_pcb* pcb = sprint_plugin_get_pcb();

  // Now it's your turn! Modify pcb->elements as desired.

  // End the plugin and send the changes back to Sprint-Layout (you can specify the merge mode here).
  sprint_require(sprint_plugin_end(SPRINT_OPERATION_REPLACE_RELATIVE));
  return 0; // Not required, as sprint_plugin_end() calls exit() internally
}
```

In addition to the PCB, some other parameters are retrieved from Sprint-Layout 
like e.g. the language: `sprint_language sprint_plugin_get_language(void);`

## License
Basically GPLv3 with an additional non-commercial use clause. Please consult LICENSE.txt for further information.

## What is missing
- Documentation
- Tests
- Sample Projects
- GUI Integration
- Configuration
- Standalone mode
