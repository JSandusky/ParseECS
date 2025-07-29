# Editor Mesh Edit

This plugin adds mesh editing support to the editor.

It demonstrates the following plugin capabilities:

- Overriding existing Property handlers
    - Used here for adding capabilities for resource properties
- Interacting with SceneViews
    - Adds a custom ViewController for detecting mesh related mouse clicks
    - Adds ViewControllers for sculpting and vertex color painting
    - Uses the Gizmo classes for manipulating vertices and faces
- Manipulating the SceneView's ToolShelf
    - Adds UI and actions for sculpting
    - Adds UI and actions for painting vertex colors