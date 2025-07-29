#pragma once

#include <GuiBuilder/DocumentBase.h>
#include <GuiBuilder/DockBuilder.h>
#include <GuiBuilder/MenuBuilder.h>
#include <GuiBuilder/RibbonBuilder.h>

#include <vector>

class DocumentManager;

class EditorApplicationBase
{
public:
    EditorApplicationBase(const std::vector<DocumentBuilder*>& documentBuilders, std::vector<DockBuilder*>& dockBuilders, std::vector<MenuBuilder*>& menuBuilders, std::vector<RibbonBuilder*>& ribbonBuilders);
    virtual ~EditorApplicationBase();

    DocumentManager* GetDocumentManager() const;


    const std::vector<DocumentBuilder*>& GetDocumentBuilders() const { return documentBuilders_; }
    const std::vector<DockBuilder*>& GetDockBuilders() const { return dockBuilders_; }
    const std::vector<MenuBuilder*>& GetMenuBuilders() const { return menuBuilders_; }
    const std::vector<RibbonBuilder*>& GetRibbonBuilders() const { return ribbonBuilders_; }

private:
    DocumentManager* documentManager_ = 0x0;
    
    std::vector<DocumentBuilder*> documentBuilders_;
    std::vector<DockBuilder*> dockBuilders_;
    std::vector<MenuBuilder*> menuBuilders_;
    std::vector<RibbonBuilder*> ribbonBuilders_;
};