#pragma once

#include <vector>

class EditorApplicationBase;
class Settings;

// Called first if we find it
typedef void(REGISTER_EDITOR_PLUGIN_SETTINGS)(Settings*);
typedef void(REGISTER_EDITOR_PLUGIN)(EditorApplicationBase*);

typedef void(PLUGIN_GENERIC_FUNCTION)(void*);

typedef const char**(REGISTER_GENERIC_PLUGIN)();
typedef const char**(REGISTER_GENERIC_PLUGIN_PARAMS)();

enum PluginParameterType
{
    PPT_Int,
    PPT_UInt,
    PPT_Float,
    PPT_String
};

struct PluginMethodInfo
{
    const char* name_;
    union {
        const char* string_;
        int integer_;
        unsigned unsigned_;
        float float_;
    } parameterDefault_;
    PluginParameterType paramType_;
};

struct Plugin
{
    const char* name_;
    const char* file_;
    const char* version_;
    REGISTER_EDITOR_PLUGIN registerPluginFunc_;
    REGISTER_GENERIC_PLUGIN registerGenericFunc_;
    REGISTER_GENERIC_PLUGIN_PARAMS genericParamFunc_;
    std::vector<PluginMethodInfo> methods_;
};

class PluginManager : protected std::vector<Plugin>
{
public:
    PluginManager(const char* pluginsFolderPath);
    virtual ~PluginManager();

    using std::vector<Plugin>::size;
    using std::vector<Plugin>::begin;
    using std::vector<Plugin>::end;
    using std::vector<Plugin>::operator[];

    /// Grabs a plugin.
    Plugin* GetPlugin(const char* name);
    /// Forcibly unloads and then reloads a plugin
    void ReloadPlugin(const char* name);
private:
};