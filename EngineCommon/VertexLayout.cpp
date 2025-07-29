#include "VertexLayout.h"

void VertexLayout::Release()
{
    if (layout_)
        layout_->Release();
    elements_.clear();
}