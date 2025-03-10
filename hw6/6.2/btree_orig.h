// From http://www.cprogramming.com/tutorial/lesson18.html 
#ifndef BTREE_H
#define BTREE_H

#include <memory>
#include <stdexcept>

template<typename T>
class btree {
private:
    struct node {
        T key_value;
        std::unique_ptr<node> left;
        std::unique_ptr<node> right;
        
        explicit node(const T& key) : key_value(key), left(nullptr), right(nullptr) {}
    };

    std::unique_ptr<node> root;

    // Helper function to deep copy a subtree
    static std::unique_ptr<node> clone_subtree(const node* src) {
        if (!src) return nullptr;
        
        auto new_node = std::make_unique<node>(src->key_value);
        // Recursively clone left and right subtrees
        if (src->left) {
            new_node->left = clone_subtree(src->left.get());
        }
        if (src->right) {
            new_node->right = clone_subtree(src->right.get());
        }
        return new_node;
    }

    void insert(const T& key, node* leaf) {
        if (key < leaf->key_value) {
            if (leaf->left) {
                insert(key, leaf->left.get());
            } else {
                leaf->left = std::make_unique<node>(key);
            }
        } else {
            if (leaf->right) {
                insert(key, leaf->right.get());
            } else {
                leaf->right = std::make_unique<node>(key);
            }
        }
    }

    const node* search(const T& key, const node* leaf) const {
        if (!leaf) return nullptr;
        
        if (key == leaf->key_value) {
            return leaf;
        }
        
        return key < leaf->key_value ? 
            search(key, leaf->left.get()) : 
            search(key, leaf->right.get());
    }

public:
    // Default constructor
    btree() = default;

    // Copy constructor - performs deep copy
    btree(const btree& other) {
        if (other.root) {
            root = clone_subtree(other.root.get());
        }
    }

    // Copy assignment - performs deep copy
    btree& operator=(const btree& other) {
        if (this != &other) {
            // Clear existing tree
            root = nullptr;
            // Copy other tree
            if (other.root) {
                root = clone_subtree(other.root.get());
            }
        }
        return *this;
    }

    // Move constructor
    btree(btree&& other) noexcept = default;

    // Move assignment
    btree& operator=(btree&& other) noexcept = default;

    // Destructor (unique_ptr handles cleanup automatically)
    ~btree() = default;

    void insert(const T& key) {
        if (root) {
            insert(key, root.get());
        } else {
            root = std::make_unique<node>(key);
        }
    }

    bool contains(const T& key) const {
        return search(key, root.get()) != nullptr;
    }

    const T* search(const T& key) const {
        const node* found = search(key, root.get());
        return found ? &found->key_value : nullptr;
    }

    bool empty() const {
        return root == nullptr;
    }

    void clear() {
        root = nullptr;
    }
};

#endif