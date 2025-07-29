#pragma once

#include "../Commands/EditorUndoStack.h"

#include <map>

class DocumentBase;

class DocumentBuilder
{
public:
    virtual DocumentBase* NewDocument() = 0;
    virtual DocumentBase* OpenDocument(const char* path) = 0;
};

class DocumentBase
{
public:
    /// Construct.
    DocumentBase(DocumentBuilder* builder);
    /// Destruct.
    virtual ~DocumentBase();

    /// Return the document builder that created this document.
    DocumentBuilder* GetBuilder() const { return builder_; }

    virtual bool SaveFile(const char* toPath) = 0;
    virtual bool IsDirty() const = 0;
    virtual void SetDirty(bool state);

    /// Called whenever this document becomes the active document.
    virtual void OnActivate() { }
    /// Called whenever this document stops being the active document.
    virtual void OnDeactive() { }
    /// DocumentBuilder should call this method immediately after opening the document.
    virtual void OnOpen() { }
    /// Called whenever this document is closed.
    virtual void OnClose() { }

protected:
    /// Contains various GUI related state variables, such as the ribbon tab that was focused, etc
    std::map<unsigned, void*> state_;
    DocumentBuilder* builder_ = 0x0;
};