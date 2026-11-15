// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.

#include "memory/memory_index.h"

// delete
void MemoryIndex::destroy(Node* n) {
    if (n == nullptr) return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

std::size_t MemoryIndex::height_of(const Node* n) {
    if (n == nullptr) return 0;
    std::size_t left = height_of(n->left);
    std::size_t right = height_of(n->right);
    return 1 + (left > right ? left : right);
}

void MemoryIndex::in_order(const Node* n, const std::function<void(const MemoryRecord&)>& fn) {
    if (n == nullptr) return;
    in_order(n->left, fn);
    fn(n->rec);
    in_order(n->right, fn);
}

// new root of a subtree after we remove id
MemoryIndex::Node* MemoryIndex::remove_node(Node* n, std::uint64_t id, bool* removed) {
    if (n == nullptr) return nullptr;

    if (id < n->rec.id) {
        n->left = remove_node(n->left, id, removed);
    } else if (id > n->rec.id) {
        n->right = remove_node(n->right, id, removed);
    } else {
        // found it
        *removed = true;
        if (n->left == nullptr && n->right == nullptr) {
            // leaf so just delete
            delete n;
            return nullptr;
        } else if (n->left == nullptr) {
            // one child on the right
            Node* child = n->right;
            delete n;
            return child;
        } else if (n->right == nullptr) {
            // one child on the left
            Node* child = n->left;
            delete n;
            return child;
        } else {
            // two children so we need the inorder successor
            Node* successor = n->right;
            while (successor->left != nullptr) successor = successor->left;
            // copy successor data into this node
            n->rec = successor->rec;
            // delete the successor node
            bool dummy = false;
            n->right = remove_node(n->right, successor->rec.id, &dummy);
        }
    }
    return n;
}

MemoryIndex::~MemoryIndex() {
    // delete the kids before the parents
    destroy(root_);
    root_ = nullptr;
}

bool MemoryIndex::insert(MemoryRecord rec) {
    // walk down to find where it goes
    Node** cur = &root_;
    while (*cur != nullptr) {
        if (rec.id == (*cur)->rec.id) return false; // already exists
        if (rec.id < (*cur)->rec.id) cur = &(*cur)->left;
        else cur = &(*cur)->right;
    }
    // fell off the tree so attach here
    *cur = new Node{std::move(rec), nullptr, nullptr};
    count_++;
    return true;
}

const MemoryRecord* MemoryIndex::find(std::uint64_t id) const {
    Node* cur = root_;
    while (cur != nullptr) {
        if (id == cur->rec.id) return &cur->rec;
        if (id < cur->rec.id) cur = cur->left;
        else cur = cur->right;
    }
    return nullptr;
}

bool MemoryIndex::remove(std::uint64_t id) {
    bool removed = false;
    root_ = remove_node(root_, id, &removed);
    if (removed) count_--;
    return removed;
}

std::size_t MemoryIndex::size() const noexcept { return count_; }

std::size_t MemoryIndex::height() const {
    return height_of(root_);
}

void MemoryIndex::for_each_in_order(const std::function<void(const MemoryRecord&)>& fn) const {
    in_order(root_, fn);
}

void MemoryIndex::clear() {
    destroy(root_);
    root_ = nullptr;
    count_ = 0;
}
