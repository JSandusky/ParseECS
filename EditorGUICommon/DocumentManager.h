#pragma once

#include <GuiBuilder/DocumentBase.h>

#include <vector>

class DocumentManager
{
public:

    DocumentBase* GetActiveDocument() const;

    std::vector<DocumentBase*>& GetOpenedDocuments() { return openedDocuments_; }
    const std::vector<DocumentBase*>& GetOpenedDocuments() const { return openedDocuments_; }

private:
    DocumentBase* activeDocument_ = 0x0;
    std::vector<DocumentBase*> openedDocuments_;
};