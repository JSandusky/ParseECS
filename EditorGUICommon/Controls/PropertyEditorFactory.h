#pragma once

class QWidget;

/// Implement to bind property data
struct PropertyEditorFactory
{
    virtual QWidget* ConstructEditor() = 0;
};