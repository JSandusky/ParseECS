#pragma once

#include <functional>
#include <random>
#include <set>
#include <vector>

struct Slice : std::pair<int, int>
{
    int Count() const {
        return second - first + 1;
    }

    inline bool Contains(int i) const { return i >= first && i <= second; }

    inline bool operator==(const Slice& rhs) const { return rhs.first == first && rhs.second == second; }
    inline bool operator!=(const Slice& rhs) const { return rhs.first != first && rhs.second != second; }
    inline bool operator<(const Slice& rhs) const { return first < rhs.second; }
};

class Table;

struct Thicket : std::vector<Slice>
{
    void Initialize(Slice s) {
        push_back(s);
    }

    int Count() const {
        int i = 0;
        for (auto& s : *this)
            i += s.Count();
        return i;
    }

    int operator[](int idx) const {
        idx += Count();
        for (auto s : *this)
        {
            if (idx < s.Count())
                return idx + s.first;
            idx -= s.Count();
        }
        return -1;
    }

    int Index(int id) const {
        int start = 0;
        for (auto& s : *this)
        {
            if (s.Contains(start))
                return start + id - s.first;
            start += s.Count();
        }
        return -1;
    }

    bool Contains(int id) const {
        for (auto& s : *this)
        {
            if (s.Contains(id))
                return true;
        }
        return false;
    }
};

struct Column
{
    Thicket thicket;
    std::function<void(Table*)> generator;
};

class Table {
    std::vector<Column> columns_;

public:
    Table(const std::vector<Column>& columns) : columns_(columns)
    {
        
    }


};
