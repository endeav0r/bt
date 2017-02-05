#ifndef plugins_HEADER
#define plugins_HEADER

#include "hooks.h"
#include "object.h"

/*
* Plugins are shared objects which provide the following three functions, all
* of which are required:
*/


/**
* Called when the plugin is first initialized. This will be called before any
* hooks are triggered.
* @return 0 on success, non-zero on failure. Failure of a plugin to initialize
*         is undefined behavior.
*/
int plugin_initialize ();

/**
* Called after all hooks are complete, and the plugin needs to cleanup.
* @return 0 on success, non-zero on failure Failure of a plugin to clean up is
*         undefined behavior.
*/
int plugin_cleanup ();

/**
* Called by the plugin manager to get the list of hooks the plugin would like to
* inject into binary toolkit.
* @return A pointer to the hooks this plugin would to inject. The hooks_api
*         should be complete and valid.
*/
const struct hooks_api * plugin_hooks ();


struct plugin {
    struct object_header oh;
    /* This is the filename of the plugin */
    const char * filename;
    /* This is the handle returned from dlopen */
    void * handle;
};

/**
* Attempts to open the plugin by calling dlopen on the filename. If dlopen
* fails, then plugin_create returns NULL.
* @param filename The filename of the plugin.
* @return A pointer to the initialized and opened plugin on success, or NULL if
*         the plugin failed to open.
*/
struct plugin * plugin_create (const char * filename);

/**
* Deletes the plugin. Also calls dlclose on the plugin's handle. Note, the
* plugin shared object may continue to be mapped into this process's memory.
* @param plugin The plugin to delete.
*/
void plugin_delete (struct plugin * plugin);

/**
* Copies the plugin, which reopens the handle to the plugin.
* @param plugin The plugin to copy.
* @return A pointer to a duplicate plugin instance.
*/
struct plugin * plugin_copy (const struct plugin * plugin);

/**
* Calls dlsym over a plugin and returns the resulting pointer.
* @param plugin The plugin to look up a symbol within.
* @param symbol The symbol to look up in this plugin.
* @return The result of the call to dlsym.
*/
void * plugin_dlsym (struct plugin * plugin, const char * symbol);


struct plugins {
    struct object_header oh;
    struct list * plugins;
};

/**
* Creates a plugins object.
* @return An empty plugins object.
*/
struct plugins * plugins_create ();

/**
* Deletes the plugins object. This will call plugin_cleanup on all plugins,
* close all plugin handles, and then cleanup.
* @param plugins The plugins object to delete.
*/
void plugins_delete (struct plugins * plugins);

/**
* Copies the plugins object. if you are doing this, you are probably headed down
* a road of terribleness and misery.
* @param plugins The plugins object to copy.
* @return A copy of the plugins object.
*
* I have changed my mind, and violated the rules of objects. There is no copy
* functin for plugins, as there simply is no clean way to copy plugins.
*/
/*
struct plugins * plugins_copy (const struct plugins * plugins);
*/

/**
* Loads a plugin and adds it to plugins_create. Loading a plugin involves the
* following steps:
* 1) Create a plugin object, which includes opening the plugin with dlopen.
* 2) Call plugin_initialize.
* 3) Register all plugin hooks.
* 4) Add the plugin to the list of tracked plugins.
* @warning Loading the same plugin twice in any single binary toolkit instance, NOT just
* plugins instance, is undefined behavior.
* @warning global_hooks_init() must be called before this function!!!
* @param plugins The initialized plugins struct.
* @param filename The filename of the plugin to load.
* @return 0 if the plugin was loaded correctly, or non-zero if the plugin failed
*         to load.
*/
int plugins_load (struct plugins * plugins, const char * filename);

#endif
