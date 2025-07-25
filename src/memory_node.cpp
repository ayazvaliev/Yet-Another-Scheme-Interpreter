#include "memory_node.h"
#include "object.h"
#include <cstddef>
#include <memory>

const std::unique_ptr<Cleaner> Cleaner::kCleaner = std::unique_ptr<Cleaner>();
int Cleaner::counter = 0;

void MemoryNode::AddDependency(MemoryNode* obj) {
    if (obj == nullptr || obj == this) {
        return;
    }
    dependencies_.push_back(obj);
    obj->upper_dependencies_.push_back(this);
}

void MemoryNode::RemoveDependency(MemoryNode* obj) {
    if (obj == nullptr || obj == this) {
        return;
    }
    for (size_t i = 0; i < dependencies_.size(); ++i) {
        if (dependencies_[i] == obj) {
            dependencies_[i] = nullptr;
        }
    }
    for (size_t i = 0; i < obj->upper_dependencies_.size(); ++i) {
        if (obj->upper_dependencies_[i] == this) {
            obj->upper_dependencies_[i] = nullptr;
        }
    }
}

void MemoryNode::SetMark() {
    marked_ = true;
    for (MemoryNode* dependants : dependencies_) {
        if (dependants == nullptr) {
            continue;
        }
        if (!dependants->marked_) {
            dependants->SetMark();
        }
    }
}

void MemoryNode::ResetMark() {
    marked_ = false;
}

void Cleaner::Sweep(MemoryNode* main_scope) {
    UnmarkAll();
    Mark(main_scope);

    for (auto& curr_obj : nodes_) {
        if (curr_obj == nullptr) {
            continue;
        }
        if (!curr_obj->marked_) {
            for (MemoryNode* upper_obj : curr_obj->upper_dependencies_) {
                if (upper_obj == nullptr) {
                    continue;
                }
                upper_obj->RemoveDependency(curr_obj);
            }
            for (MemoryNode* lower_obj : curr_obj->dependencies_) {
                if (lower_obj == nullptr) {
                    continue;
                }
                curr_obj->RemoveDependency(lower_obj);
            }
        }
        delete curr_obj;
        curr_obj = nullptr;
    }
}

Scope* Cleaner::MakeScope(Scope* parent_scope) {
    auto new_scope = new Scope(parent_scope);
    nodes_.push_back(new_scope);
    return new_scope;
}

void Cleaner::Mark(MemoryNode* main_scope) {
    main_scope->SetMark();
}

void Cleaner::UnmarkAll() {
    for (auto& obj : nodes_) {
        if (obj == nullptr) {
            continue;
        }
        obj->ResetMark();
    }
}

void Cleaner::DeleteAll() {
    for (auto& obj : nodes_) {
        delete obj;
    }
}