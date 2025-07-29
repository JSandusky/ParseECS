#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace EditorCommon
{

    class MemoryVisualizer
    {
    public:

        struct MemorySpan
        {
            unsigned tag_;
            unsigned offset_;
            unsigned length_;
        };

        struct SpanEnumerator
        {
            virtual std::string GetName() const = 0;
            virtual void GetSpans(std::vector<MemorySpan>& spans) = 0;
        };

        void AddSpanEnumerator(std::shared_ptr<SpanEnumerator>);
        void RemoveSpanEnumerator(std::shared_ptr<SpanEnumerator>);
        void ClearSpanEnumerators();
        void SetTagname(unsigned tag, const std::string& name);

    private:
        std::map<unsigned, std::string> tagNames_;
        std::vector< std::pair<std::shared_ptr<SpanEnumerator>, std::vector<MemorySpan> > > spanEnumerators_;
    };

}