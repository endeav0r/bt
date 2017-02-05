#include "plugins.h"

#include "btlog.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>


const struct object_vtable plugin_vtable = {
    (void (*) (void *)) plugin_delete,
    (void * (*) (const void *)) plugin_copy,
    NULL
};


struct plugin * plugin_create (const char * filename) {
    void * handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        btlog("[plugins.create] dlopen for %s failed", filename);
        return NULL;
    }

    struct plugin * plugin = malloc(sizeof(struct plugin));
    object_init(plugin, &plugin_vtable);
    plugin->filename = strdup(filename);
    btlog("plugin->filename = %p", plugin->filename);
    plugin->handle = handle;
    return plugin;
}


void plugin_delete (struct plugin * plugin) {
    btlog("[plugin_delete]");
    dlclose(plugin->handle);
    free((void *) plugin->filename);
    free(plugin);
}


struct plugin * plugin_copy (const struct plugin * plugin) {
    /* TODO Oh boy, this can return NULL, which is bad. */
    btlog("[plugin_copy]");
    return plugin_create(plugin->filename);
}


void * plugin_dlsym (struct plugin * plugin, const char * symbol) {
    return dlsym(plugin->handle, symbol);
}


const struct object_vtable plugins_vtable = {
    (void (*) (void *)) plugins_delete,
    NULL,
    NULL
};


struct plugins * plugins_create () {
    struct plugins * plugins = malloc(sizeof(struct plugins));
    object_init(plugins, &plugins_vtable);
    plugins->plugins = list_create();
    return plugins;
}


void plugins_delete (struct plugins * plugins) {
    struct list_it * it;
    for (it = list_it(plugins->plugins); it != NULL; it = list_it_next(it)) {
        struct plugin * plugin = list_it_data(it);
        typeof(plugin_cleanup) * pc = plugin_dlsym(plugin, "plugin_cleanup");
        if (pc != NULL) {
            btlog("[plugins_delete] calling plugin_cleanup for %p %p",
                  plugin->filename, pc);
            pc();
        }
        else
            btlog("[plugins.delete] %s has no plugin_cleanup",
                  plugin->filename);
    }
    ODEL(plugins->plugins);
    free(plugins);
}


int plugins_load (struct plugins * plugins, const char * filename) {
    struct plugin * plugin = plugin_create(filename);
    if (plugin == NULL)
        return -1;

    typeof(plugin_initialize) * pi = plugin_dlsym(plugin, "plugin_initialize");
    typeof(plugin_hooks) * ph = plugin_dlsym(plugin, "plugin_hooks");
    if ((pi == NULL) || (ph == NULL)) {
        ODEL(plugin);
        return -1;
    }
    pi();

    const struct hooks_api * hooks_api = ph();
    global_hooks_append(hooks_api);

    list_append_(plugins->plugins, plugin);

    btlog("[plugins_load] %s loaded", plugin->filename);

    return 0;
}
