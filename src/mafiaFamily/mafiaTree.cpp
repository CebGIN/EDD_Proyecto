#include "mafiaTree.hpp"

// =========================================================================
// Constructor / Destructor
// =========================================================================

MafiaTree::MafiaTree() : BinaryTree<FamilyMember>(), parentMap(nullptr) {}

MafiaTree::~MafiaTree() {
  clearParentMap();
  // Base class ~BinaryTree<>() handles the tree nodes.
}

// =========================================================================
// Parent-map helpers (replaces std::unordered_map — no vectors allowed)
// =========================================================================

void MafiaTree::buildParentMap(BTNode<FamilyMember> *node,
                               BTNode<FamilyMember> *parent) {
  if (!node)
    return;

  ParentEntry *entry = new ParentEntry{node, parent, parentMap};
  parentMap = entry;

  buildParentMap(node->left, node);
  buildParentMap(node->right, node);
}

void MafiaTree::clearParentMap() {
  ParentEntry *current = parentMap;
  while (current) {
    ParentEntry *next = current->next;
    delete current;
    current = next;
  }
  parentMap = nullptr;
}

BTNode<FamilyMember> *MafiaTree::getParentOf(BTNode<FamilyMember> *node) const {
  ParentEntry *entry = parentMap;
  while (entry) {
    if (entry->node == node)
      return entry->parent;
    entry = entry->next;
  }
  return nullptr;
}

// =========================================================================
// ID-based search
// =========================================================================

BTNode<FamilyMember> *MafiaTree::findById(BTNode<FamilyMember> *node,
                                          int id) const {
  if (!node)
    return nullptr;
  if (node->data.id == id)
    return node;

  BTNode<FamilyMember> *leftResult = findById(node->left, id);
  if (leftResult)
    return leftResult;

  return findById(node->right, id);
}

FamilyMember *MafiaTree::findMemberById(int id) {
  BTNode<FamilyMember> *node = findById(root, id);
  return node ? &node->data : nullptr;
}

BTNode<FamilyMember> *MafiaTree::getBossNode() const {
  // Walk the tree to find the node marked as current boss.
  // We perform a level-order-style pre-order search.
  if (!root)
    return nullptr;

  // Use a simple stack via the parentMap structure already built,
  // but since parentMap may not be built yet, use recursion.
  struct BossSearcher {
    static BTNode<FamilyMember> *find(BTNode<FamilyMember> *n) {
      if (!n)
        return nullptr;
      if (n->data.is_boss)
        return n;
      BTNode<FamilyMember> *l = find(n->left);
      if (l)
        return l;
      return find(n->right);
    }
  };
  return BossSearcher::find(root);
}

// =========================================================================
// Tree construction
// =========================================================================

bool MafiaTree::insertUnder(BTNode<FamilyMember> *node,
                            const FamilyMember &member) {
  if (!node)
    return false;

  if (node->data.id == member.id_boss) {
    // First slot: left child
    if (!node->left) {
      node->left = new BTNode<FamilyMember>(member);
      return true;
    }
    // Second slot: right child
    if (!node->right) {
      node->right = new BTNode<FamilyMember>(member);
      return true;
    }
    // Both slots taken — this CSV data would violate the binary tree
    // constraint. Per specs, each boss has at most 2 successors; we silently
    // drop extra entries.
    return false;
  }

  // Recurse
  if (insertUnder(node->left, member))
    return true;
  return insertUnder(node->right, member);
}

void MafiaTree::buildFromList(const cde::LinkedList<FamilyMember> *members) {
  if (!members || members->get_size() == 0)
    return;

  // Clear any existing tree
  clear();
  clearParentMap();

  int size = members->get_size();

  // --- Pass 1: find the root (id_boss == 0) ---
  for (int i = 0; i < size; ++i) {
    FamilyMember m = members->get(i);
    if (m.id_boss == 0) {
      root = new BTNode<FamilyMember>(m);
      break;
    }
  }

  if (!root)
    return; // malformed data

  // --- Pass 2: insert remaining members ---
  // We may need multiple passes if children appear before their parents in the
  // CSV.
  bool inserted = true;
  while (inserted) {
    inserted = false;
    for (int i = 0; i < size; ++i) {
      FamilyMember m = members->get(i);
      if (m.id_boss == 0)
        continue; // already placed as root
      if (findById(root, m.id))
        continue; // already in the tree

      if (insertUnder(root, m)) {
        inserted = true;
      }
    }
  }

  // --- Build parent map for succession traversal ---
  buildParentMap(root, nullptr);
}

// =========================================================================
// Succession line
// =========================================================================

void MafiaTree::getSuccessionLine(cde::LinkedList<FamilyMember> &out) const {
  // Pre-order traversal — collects only ALIVE members.
  struct Collector {
    static void collect(BTNode<FamilyMember> *n,
                        cde::LinkedList<FamilyMember> &list) {
      if (!n)
        return;
      if (!n->data.is_dead) {
        list.push_back(n->data);
      }
      collect(n->left, list);
      collect(n->right, list);
    }
  };
  Collector::collect(root, out);
}

// =========================================================================
// Succession helpers
// =========================================================================

BTNode<FamilyMember> *MafiaTree::findFirstAvailable(BTNode<FamilyMember> *node,
                                                    bool ignoreJail) const {
  if (!node)
    return nullptr;
  if (!node->data.is_dead && (ignoreJail || !node->data.in_jail))
    return node;

  BTNode<FamilyMember> *leftResult = findFirstAvailable(node->left, ignoreJail);
  if (leftResult)
    return leftResult;

  return findFirstAvailable(node->right, ignoreJail);
}

BTNode<FamilyMember> *
MafiaTree::findNearestBossWithTwoSuccessors(BTNode<FamilyMember> *bossNode,
                                            bool ignoreJail) const {
  // Walk up through parents, looking for an ancestor that has both left and
  // right children, with workable candidates on BOTH sides.
  BTNode<FamilyMember> *current = getParentOf(bossNode);
  while (current) {
    if (current->left && current->right) {
      // Check if both sides have a workable candidate
      BTNode<FamilyMember> *cLeft =
          findFirstAvailable(current->left, ignoreJail);
      BTNode<FamilyMember> *cRight =
          findFirstAvailable(current->right, ignoreJail);
      if (cLeft && cRight)
        return current;
    }
    current = getParentOf(current);
  }
  return nullptr;
}

// =========================================================================
// Succession promotion
// =========================================================================

void MafiaTree::evaluateSuccession() {
  BTNode<FamilyMember> *bossNode = getBossNode();
  if (!bossNode)
    return;

  FamilyMember &boss = bossNode->data;

  // Check if succession must trigger
  bool mustReplace = boss.is_dead || boss.in_jail || boss.age >= 70;
  if (!mustReplace)
    return;

  BTNode<FamilyMember> *newBossNode = nullptr;

  auto findProcess = [&](bool ignoreJail) -> BTNode<FamilyMember> * {
    BTNode<FamilyMember> *potentialBoss = nullptr;
    // Rule 3.1
    if (bossNode->left || bossNode->right) {
      potentialBoss = findFirstAvailable(bossNode->left, ignoreJail);
      if (!potentialBoss)
        potentialBoss = findFirstAvailable(bossNode->right, ignoreJail);
    }

    // Rule 3.2 & 3.3
    if (!potentialBoss) {
      BTNode<FamilyMember> *grandParent = getParentOf(bossNode);
      if (grandParent) {
        BTNode<FamilyMember> *partner = (grandParent->left == bossNode)
                                            ? grandParent->right
                                            : grandParent->left;
        if (partner) {
          potentialBoss = findFirstAvailable(partner->left, ignoreJail);
          if (!potentialBoss)
            potentialBoss = findFirstAvailable(partner->right, ignoreJail);
          if (!potentialBoss && !partner->left && !partner->right &&
              !partner->data.is_dead &&
              (ignoreJail || !partner->data.in_jail)) {
            potentialBoss = partner;
          }
        }
      }
    }

    // Rule 3.4
    if (!potentialBoss) {
      BTNode<FamilyMember> *grandParent = getParentOf(bossNode);
      BTNode<FamilyMember> *greatGrandParent =
          grandParent ? getParentOf(grandParent) : nullptr;
      if (greatGrandParent) {
        BTNode<FamilyMember> *uncle = (greatGrandParent->left == grandParent)
                                          ? greatGrandParent->right
                                          : greatGrandParent->left;
        if (uncle) {
          potentialBoss = findFirstAvailable(uncle->left, ignoreJail);
          if (!potentialBoss)
            potentialBoss = findFirstAvailable(uncle->right, ignoreJail);
          if (!potentialBoss && !uncle->left && !uncle->right &&
              !uncle->data.is_dead && (ignoreJail || !uncle->data.in_jail)) {
            potentialBoss = uncle;
          }
        }
      }
    }

    // Rule 3.5
    if (!potentialBoss) {
      BTNode<FamilyMember> *ancestor =
          findNearestBossWithTwoSuccessors(bossNode, ignoreJail);
      if (ancestor) {
        potentialBoss = findFirstAvailable(ancestor->left, ignoreJail);
        if (!potentialBoss)
          potentialBoss = findFirstAvailable(ancestor->right, ignoreJail);
      }
    }
    return potentialBoss;
  };

  newBossNode = findProcess(false);

  if (!newBossNode) {
    struct GlobalFree {
      static BTNode<FamilyMember> *find(BTNode<FamilyMember> *n) {
        if (!n)
          return nullptr;
        if (!n->data.is_dead && !n->data.in_jail)
          return n;
        BTNode<FamilyMember> *l = find(n->left);
        if (l)
          return l;
        return find(n->right);
      }
    };
    newBossNode = GlobalFree::find(root);
  }

  if (!newBossNode) {
    newBossNode = findProcess(true);
  }

  if (!newBossNode) {
    struct GlobalJail {
      static BTNode<FamilyMember> *find(BTNode<FamilyMember> *n) {
        if (!n)
          return nullptr;
        if (!n->data.is_dead)
          return n;
        BTNode<FamilyMember> *l = find(n->left);
        if (l)
          return l;
        return find(n->right);
      }
    };
    newBossNode = GlobalJail::find(root);
  }

  if (!newBossNode || newBossNode == bossNode)
    return;

  boss.is_boss = false;
  if (!boss.is_dead)
    boss.was_boss = true;

  newBossNode->data.is_boss = true;
  newBossNode->data.was_boss = false;
}

// =========================================================================
// Modification
// =========================================================================

bool MafiaTree::updateMember(int id, const std::string &name,
                             const std::string &last_name, char gender, int age,
                             bool is_dead, bool in_jail) {
  FamilyMember *member = findMemberById(id);
  if (!member)
    return false;

  member->name = name;
  member->last_name = last_name;
  member->gender = gender;
  member->age = age;
  member->is_dead = is_dead;
  member->in_jail = in_jail;

  // Evaluate succession if the updated member is the boss OR if the current
  // boss is unfit
  BTNode<FamilyMember> *bossNode = getBossNode();
  if (member->is_boss ||
      (bossNode && (bossNode->data.in_jail || bossNode->data.is_dead ||
                    bossNode->data.age >= 70))) {
    evaluateSuccession();
  }

  return true;
}

// =========================================================================
// Serialization
// =========================================================================

void MafiaTree::toList(cde::LinkedList<FamilyMember> &out) const {
  // Pre-order traversal
  struct Serializer {
    static void collect(BTNode<FamilyMember> *n,
                        cde::LinkedList<FamilyMember> &list) {
      if (!n)
        return;
      list.push_back(n->data);
      collect(n->left, list);
      collect(n->right, list);
    }
  };
  Serializer::collect(root, out);
}

void MafiaTree::toIndentedList(cde::LinkedList<std::string> &out) const {
  struct Indenter {
    static void collect(BTNode<FamilyMember> *n, int depth,
                        cde::LinkedList<std::string> &list) {
      if (!n)
        return;

      std::string indent = "";
      for (int i = 0; i < depth; ++i) {
        indent += "  ";
      }

      std::string status = "";
      if (n->data.is_boss)
        status = " [BOSS]";
      else if (n->data.is_dead)
        status = " [MUERTO]";
      else if (n->data.in_jail)
        status = " [CARCEL]";

      std::string line = indent + "|-- " + n->data.name + " " +
                         n->data.last_name +
                         " (ID: " + std::to_string(n->data.id) + ")" + status;
      list.push_back(line);

      collect(n->left, depth + 1, list);
      collect(n->right, depth + 1, list);
    }
  };
  Indenter::collect(root, 0, out);
}
