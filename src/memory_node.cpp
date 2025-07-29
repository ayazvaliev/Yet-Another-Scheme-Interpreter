#include "memory_node.h"
#include "object.h"
#include <memory>

const std::unique_ptr<Cleaner> Cleaner::kCleaner = std::make_unique<Cleaner>();
int Cleaner::counter = 0;

void MemoryNode::AddDependency(MemoryNode* obj) {
    if (obj == nullptr || obj == this || dependencies_.contains(obj)) {
            return;
        }
    dependencies_.insert(obj);
    obj->upper_dependencies_.insert(this);
}

void MemoryNode::RemoveDependency(MemoryNode* obj) {
   if (obj == nullptr || obj == this) {
            return;
        }
    dependencies_.erase(obj);
    obj->upper_dependencies_.erase(this);
}

void MemoryNode::SetMark() {
    marked_ = true;
    for (MemoryNode* dependants : dependencies_) {
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

    std::vector<MemoryNode*> new_nodes;
    for (auto& curr_obj : nodes_) {
        if (curr_obj == nullptr) {
            continue;
        }
        if (!curr_obj->marked_) {
            for (MemoryNode* upper_obj : curr_obj->upper_dependencies_) {
                upper_obj->dependencies_.erase(curr_obj);     
            }
            for (MemoryNode* lower_obj : curr_obj->dependencies_) {
                lower_obj->upper_dependencies_.erase(curr_obj);
            }
            delete curr_obj;
        } else {
            new_nodes.push_back(curr_obj);
        }
    }
    nodes_ = std::move(new_nodes);
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