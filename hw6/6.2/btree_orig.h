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
        
        // Copy constructor for node
        node(const node& other) : 
            key_value(other.key_value),
            left(other.left ? std::make_unique<node>(*other.left) : nullptr),
            right(other.right ? std::make_unique<node>(*other.right) : nullptr) {}
    };

    std::unique_ptr<node> root;

    // Helper function to copy nodes recursively
    std::unique_ptr<node> copy_tree(const node* src) {
        if (!src) return nullptr;
        return std::make_unique<node>(*src);
    }

    // Helper function to insert a key into a subtree
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

    // Helper function to search for a key in a subtree
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

    // Copy constructor
    btree(const btree& other) {
        if (other.root) {
            root = copy_tree(other.root.get());
        }
    }

    // Move constructor
    btree(btree&& other) noexcept = default;

    // Copy assignment operator
    btree& operator=(const btree& other) {
        if (this != &other) {
            btree temp(other);
            std::swap(root, temp.root);
        }
        return *this;
    }

    // Move assignment operator
    btree& operator=(btree&& other) noexcept = default;

    // Destructor (unique_ptr handles cleanup automatically)
    ~btree() = default;

    // Insert a key into the tree
    void insert(const T& key) {
        if (root) {
            insert(key, root.get());
        } else {
            root = std::make_unique<node>(key);
        }
    }

    // Search for a key in the tree
    bool search(const T& key) const {
        return search(key, root.get()) != nullptr;
    }

    // Get value at key (throws if not found)
    const T& at(const T& key) const {
        const node* found = search(key, root.get());
        if (!found) {
            throw std::out_of_range("Key not found in tree");
        }
        return found->key_value;
    }

    // Check if tree is empty
    bool empty() const {
        return root == nullptr;
    }

    // Clear the tree
    void clear() {
        root.reset();
    }
};

#endif