#ifndef BINARY_TREE_HPP
#define BINARY_TREE_HPP

#include <functional>
#include <string>
/**
 * @brief A generic Node structure for a Binary Tree.
 * @tparam T The type of data stored in the node.
 */
template <typename T>
struct BTNode {
    T data;
    BTNode<T>* left;
    BTNode<T>* right;

    BTNode(const T& value);
};

/**
 * @brief A generic Binary Tree implementation.
 */
template <typename T>
class BinaryTree {
protected:
    BTNode<T>* root;

    // Helper functions for recursive operations
    void clear(BTNode<T>* node);
    BTNode<T>* copy(BTNode<T>* node);
    int getHeight(BTNode<T>* node) const;
    size_t getSize(BTNode<T>* node) const;

    // Traversals
    void inOrder(BTNode<T>* node, std::function<void(T&)> callback);
    void preOrder(BTNode<T>* node, std::function<void(T&)> callback);
    void postOrder(BTNode<T>* node, std::function<void(T&)> callback);

    // Helpers
    bool isBalanced(BTNode<T>* node) const;
    void buildString(BTNode<T>* node, std::string prefix, bool isLeft, std::string& result) const;

public:
    BinaryTree();
    virtual ~BinaryTree();

    // Rule of Three
    BinaryTree(const BinaryTree& other);
    BinaryTree& operator=(const BinaryTree& other);

    // Basic Properties
    bool isEmpty() const;
    int getHeight() const;
    size_t getSize() const;
    BTNode<T>* getRoot() const;
    bool isBalanced() const;
    int getBalanceFactor(BTNode<T>* node) const;
    void clear();

    std::string toString() const;

    // Traversals
    void traverseInOrder(std::function<void(T&)> callback);
    void traversePreOrder(std::function<void(T&)> callback);
    void traversePostOrder(std::function<void(T&)> callback);
    void traverseLevelOrder(std::function<void(T&)> callback);
};

/**
 * @brief A generic Binary Search Tree (BST) implementation.
 */
template <typename T>
class BinarySearchTree : public BinaryTree<T> {
protected:
    BTNode<T>* insert(BTNode<T>* node, const T& value);
    BTNode<T>* find(BTNode<T>* node, const T& value) const;
    BTNode<T>* findMin(BTNode<T>* node) const;
    BTNode<T>* remove(BTNode<T>* node, const T& value);

public:
    BinarySearchTree();

    void insert(const T& value);
    bool contains(const T& value) const;
    void remove(const T& value);
    const T* search(const T& value) const;
};

// Include implementation for templates
#include "binaryTree.cpp"

#endif // BINARY_TREE_HPP
