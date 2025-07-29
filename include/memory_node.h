#pragma once

#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_set>

class Object;
class Cleaner;
class Scope;


class MemoryNode {
public:
    friend Cleaner;

    MemoryNode() = default;
    virtual ~MemoryNode() = default;
protected:
    void AddDependency(MemoryNode* obj);
    void RemoveDependency(MemoryNode* obj);
    void SetMark();
    void ResetMark();

    std::unordered_set<MemoryNode*> dependencies_;
    std::unordered_set<MemoryNode*> upper_dependencies_;
    bool marked_;

};


class Cleaner {
public:
    Cleaner() = default;

    template<typename T, typename... Args>
    requires std::is_base_of_v<MemoryNode, T>
    Object* Make(Args... args) {
        auto new_obj = new T(std::forward<Args>(args)...);
        nodes_.push_back(static_cast<MemoryNode*>(new_obj));
        return new_obj;
    }

    Scope* MakeScope(Scope* parent_scope);

    void Sweep(MemoryNode* main_scope);
    void DeleteAll();

    static const std::unique_ptr<Cleaner> kCleaner;
    static int counter;

private:
    void Mark(MemoryNode* main_scope);
    void UnmarkAll();
    std::vector<MemoryNode*> nodes_;
};