# EditorGUICommon

The contents of this library are used by the main EditorGUI program and by plugins. 

## Writing Plugins

Plugins should link to the EditorGUICommon DLL. All they though don't necessarilly have to there are several things that can only be done by implementing interfaces from here:

- Use QSexyToolTip for consistent flyouts
- Use the Node editor on your data
- Use the timeline widget on your data
- Show your action commands in the *Quick Action* menu
- Creating new:
    - Core GUI
        - Docking panels
        - menu commands
        - Ribbon tabs
    - Documents and Datasources
    - Property pages for new datasources
    - File preview renderers

Plugins that do not link to this DLL are limited to some basic command handling in that they can register functions with a *void MyFunction(void\*)* signature through an implementation of a *const char\*\* GetExportedFunctions()* function.

### Implementing an EditorGUICommon linking Editor plugin

Export a function precisely named *void GetEditorPluginInformation(unsigned\* version, unsigned\* capabilities)* if this function is not found then it will be assumed that the plugin is a loose style plugin.

Implement the **following** functions as desired:

*void InitializeEditorPlugin(ApplicationBase\*)*

### Implmenting a loose Editor plugin

The plugin must export a function *const char\*\* GetExportedFunctions()* which should return a table of the names for all exported functions to bind. The last element should be 0x0;

These exported functions need to have a signature of *void MyFunction(void\*)*

If **any** of these exported functions needs argument data fed to it, then you must implement a *const char\*\* GetExportedFunctionParams()* that returns a table with three entries per function. The first record must be the display name to present the argument as, the second must be the type to use, and the third must be a default value for the argument. Like with the function list table the last value should be 0x0.

Supported argument types are:

- **string** : strings (UTF8)
- **int** : integers
- **uint** : unsigned integers
- **float** : 32bit floating point

Here is an example implemention of GetExportedFunctions():

    static const char** functionTable_ = {
        "RunReportsBatch",
        "OpenTwine",
        0x0
    };

    // We'll use a struct to make it more convenient to write
    struct {
        const char* display;
        const char* type;
        const char* defaultValue;
    } parametersTable_ = {
        {  "", "", "" },                // RunReportsBatch doesn't use the parameter
        { "File path", "string", "" },  // OpenTwine uses a string parameters
        { 0x0, 0x0, 0x0 }
    };

    const char** GetExportedFunctions()
    {
        return functionTable_;
    }

    const char** GetExportedFunctionParams()
    {
        return (const char**)&parametersTable_;
    }

    void RunReportsBatch(void* /*unused*/)
    {
        // do work
    }

    void OpenTwine(void* param)
    {
        const char* filePath = (const char*)param;
    }
