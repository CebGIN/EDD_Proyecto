#ifndef BINARY_TREE_CPP
#define BINARY_TREE_CPP

#ifndef BINARY_TREE_HPP
#include "binaryTree.hpp"
#endif

#include <queue>
#include <algorithm>
#include <cmath>

// =====================================================================
// BTNode Implementation
// =====================================================================

template <typename T>
BTNode<T>::BTNode(const T &value)
    : data(value), left(nullptr), right(nullptr) {}

// =====================================================================
// BinaryTree Implementation
// =====================================================================

template <typename T> BinaryTree<T>::BinaryTree() : root(nullptr) {}

template <typename T> BinaryTree<T>::~BinaryTree() { clear(root); }

template <typename T> BinaryTree<T>::BinaryTree(const BinaryTree &other) {
  root = copy(other.root);
}

template <typename T>
BinaryTree<T> &BinaryTree<T>::operator=(const BinaryTree &other) {
  if (this != &other) {
    clear(root);
    root = copy(other.root);
  }
  return *this;
}

template <typename T> void BinaryTree<T>::clear(BTNode<T> *node) {
  if (node) {
    clear(node->left);
    clear(node->right);
    delete node;
  }
}

template <typename T> BTNode<T> *BinaryTree<T>::copy(BTNode<T> *node) {
  if (!node)
    return nullptr;
  BTNode<T> *newNode = new BTNode<T>(node->data);
  newNode->left = copy(node->left);
  newNode->right = copy(node->right);
  return newNode;
}

template <typename T> int BinaryTree<T>::getHeight(BTNode<T> *node) const {
  if (!node)
    return 0;
  return 1 + std::max(getHeight(node->left), getHeight(node->right));
}

template <typename T> size_t BinaryTree<T>::getSize(BTNode<T> *node) const {
  if (!node)
    return 0;
  return 1 + getSize(node->left) + getSize(node->right);
}

template <typename T>
void BinaryTree<T>::inOrder(BTNode<T> *node,
                            std::function<void(T &)> callback) {
  if (node) {
    inOrder(node->left, callback);
    callback(node->data);
    inOrder(node->right, callback);
  }
}

template <typename T>
void BinaryTree<T>::preOrder(BTNode<T> *node,
                             std::function<void(T &)> callback) {
  if (node) {
    callback(node->data);
    preOrder(node->left, callback);
    preOrder(node->right, callback);
  }
}

template <typename T>
void BinaryTree<T>::postOrder(BTNode<T> *node,
                              std::function<void(T &)> callback) {
  if (node) {
    postOrder(node->left, callback);
    postOrder(node->right, callback);
    callback(node->data);
  }
}

template <typename T> bool BinaryTree<T>::isBalanced(BTNode<T> *node) const {
  if (!node)
    return true;
  int leftHeight = getHeight(node->left);
  int rightHeight = getHeight(node->right);
  return std::abs(leftHeight - rightHeight) <= 1 && isBalanced(node->left) &&
         isBalanced(node->right);
}

template <typename T>
void BinaryTree<T>::buildString(BTNode<T> *node, std::string prefix,
                                bool isLeft, std::string &result) const {
  if (!node)
    return;
  result += prefix;
  result += (isLeft ? "├──" : "└──");
  result += std::to_string(node->data) + "\n";
  buildString(node->left, prefix + (isLeft ? "│   " : "    "), true, result);
  buildString(node->right, prefix + (isLeft ? "│   " : "    "), false, result);
}

template <typename T> bool BinaryTree<T>::isEmpty() const {
  return root == nullptr;
}

template <typename T> int BinaryTree<T>::getHeight() const {
  return getHeight(root);
}

template <typename T> size_t BinaryTree<T>::getSize() const {
  return getSize(root);
}

template <typename T> BTNode<T> *BinaryTree<T>::getRoot() const { return root; }

template <typename T> bool BinaryTree<T>::isBalanced() const {
  return isBalanced(root);
}

template <typename T>
int BinaryTree<T>::getBalanceFactor(BTNode<T> *node) const {
  if (!node)
    return 0;
  return getHeight(node->left) - getHeight(node->right);
}

template <typename T> void BinaryTree<T>::clear() {
  clear(root);
  root = nullptr;
}

template <typename T> std::string BinaryTree<T>::toString() const {
  std::string result;
  buildString(root, "", false, result);
  return result;
}

template <typename T>
void BinaryTree<T>::traverseInOrder(std::function<void(T &)> callback) {
  inOrder(root, callback);
}

template <typename T>
void BinaryTree<T>::traversePreOrder(std::function<void(T &)> callback) {
  preOrder(root, callback);
}

template <typename T>
void BinaryTree<T>::traversePostOrder(std::function<void(T &)> callback) {
  postOrder(root, callback);
}

template <typename T>
void BinaryTree<T>::traverseLevelOrder(std::function<void(T &)> callback) {
  if (!root)
    return;
  std::queue<BTNode<T> *> q;
  q.push(root);
  while (!q.empty()) {
    BTNode<T> *current = q.front();
    q.pop();
    callback(current->data);
    if (current->left)
      q.push(current->left);
    if (current->right)
      q.push(current->right);
  }
}

// =====================================================================
// BinarySearchTree Implementation
// =====================================================================

template <typename T>
BinarySearchTree<T>::BinarySearchTree() : BinaryTree<T>() {}

template <typename T>
BTNode<T> *BinarySearchTree<T>::insert(BTNode<T> *node, const T &value) {
  if (!node)
    return new BTNode<T>(value);
  if (value < node->data)
    node->left = insert(node->left, value);
  else if (value > node->data)
    node->right = insert(node->right, value);
  return node;
}

template <typename T>
BTNode<T> *BinarySearchTree<T>::find(BTNode<T> *node, const T &value) const {
  if (!node || node->data == value)
    return node;
  if (value < node->data)
    return find(node->left, value);
  return find(node->right, value);
}

template <typename T>
BTNode<T> *BinarySearchTree<T>::findMin(BTNode<T> *node) const {
  while (node && node->left)
    node = node->left;
  return node;
}

template <typename T>
BTNode<T> *BinarySearchTree<T>::remove(BTNode<T> *node, const T &value) {
  if (!node)
    return nullptr;

  if (value < node->data)
    node->left = remove(node->left, value);
  else if (value > node->data)
    node->right = remove(node->right, value);
  else {
    if (!node->left) {
      BTNode<T> *temp = node->right;
      delete node;
      return temp;
    } else if (!node->right) {
      BTNode<T> *temp = node->left;
      delete node;
      return temp;
    }
    BTNode<T> *temp = findMin(node->right);
    node->data = temp->data;
    node->right = remove(node->right, temp->data);
  }
  return node;
}

template <typename T> void BinarySearchTree<T>::insert(const T &value) {
  this->root = insert(this->root, value);
}

template <typename T> bool BinarySearchTree<T>::contains(const T &value) const {
  return find(this->root, value) != nullptr;
}

template <typename T> void BinarySearchTree<T>::remove(const T &value) {
  this->root = remove(this->root, value);
}

template <typename T>
const T *BinarySearchTree<T>::search(const T &value) const {
  BTNode<T> *result = find(this->root, value);
  return result ? &result->data : nullptr;
}

#endif // BINARY_TREE_CPP
