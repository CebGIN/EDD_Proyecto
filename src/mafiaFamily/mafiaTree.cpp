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

void MafiaTree::buildParentMap(BTNode<FamilyMember>* node, BTNode<FamilyMember>* parent) {
    if (!node) return;

    ParentEntry* entry = new ParentEntry{node, parent, parentMap};
    parentMap = entry;

    buildParentMap(node->left, node);
    buildParentMap(node->right, node);
}

void MafiaTree::clearParentMap() {
    ParentEntry* current = parentMap;
    while (current) {
        ParentEntry* next = current->next;
        delete current;
        current = next;
    }
    parentMap = nullptr;
}

BTNode<FamilyMember>* MafiaTree::getParentOf(BTNode<FamilyMember>* node) const {
    ParentEntry* entry = parentMap;
    while (entry) {
        if (entry->node == node) return entry->parent;
        entry = entry->next;
    }
    return nullptr;
}

// =========================================================================
// ID-based search
// =========================================================================

BTNode<FamilyMember>* MafiaTree::findById(BTNode<FamilyMember>* node, int id) const {
    if (!node) return nullptr;
    if (node->data.id == id) return node;

    BTNode<FamilyMember>* leftResult = findById(node->left, id);
    if (leftResult) return leftResult;

    return findById(node->right, id);
}

FamilyMember* MafiaTree::findMemberById(int id) {
    BTNode<FamilyMember>* node = findById(root, id);
    return node ? &node->data : nullptr;
}

BTNode<FamilyMember>* MafiaTree::getBossNode() const {
    // Walk the tree to find the node marked as current boss.
    // We perform a level-order-style pre-order search.
    if (!root) return nullptr;

    // Use a simple stack via the parentMap structure already built,
    // but since parentMap may not be built yet, use recursion.
    struct BossSearcher {
        static BTNode<FamilyMember>* find(BTNode<FamilyMember>* n) {
            if (!n) return nullptr;
            if (n->data.is_boss) return n;
            BTNode<FamilyMember>* l = find(n->left);
            if (l) return l;
            return find(n->right);
        }
    };
    return BossSearcher::find(root);
}

// =========================================================================
// Tree construction
// =========================================================================

bool MafiaTree::insertUnder(BTNode<FamilyMember>* node, const FamilyMember& member) {
    if (!node) return false;

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
        // Both slots taken — this CSV data would violate the binary tree constraint.
        // Per specs, each boss has at most 2 successors; we silently drop extra entries.
        return false;
    }

    // Recurse
    if (insertUnder(node->left, member)) return true;
    return insertUnder(node->right, member);
}

void MafiaTree::buildFromList(const cde::LinkedList<FamilyMember>* members) {
    if (!members || members->get_size() == 0) return;

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

    if (!root) return; // malformed data

    // --- Pass 2: insert remaining members ---
    // We may need multiple passes if children appear before their parents in the CSV.
    bool inserted = true;
    while (inserted) {
        inserted = false;
        for (int i = 0; i < size; ++i) {
            FamilyMember m = members->get(i);
            if (m.id_boss == 0) continue;           // already placed as root
            if (findById(root, m.id)) continue;     // already in the tree

            if (insertUnder(root, m)) {
                inserted = true;
            }
        }
    }

    // --- Build parent map for succession traversal ---
    buildParentMap(root, nullptr);
}

// =========================================================================
// Succession line (requirement 2)
// =========================================================================

void MafiaTree::getSuccessionLine(cde::LinkedList<FamilyMember>& out) const {
    // Pre-order traversal — collects only ALIVE and NOT IN JAIL members.
    struct Collector {
        static void collect(BTNode<FamilyMember>* n, cde::LinkedList<FamilyMember>& list) {
            if (!n) return;
            if (!n->data.is_dead && !n->data.in_jail) {
                list.push_back(n->data);
            }
            collect(n->left, list);
            collect(n->right, list);
        }
    };
    Collector::collect(root, out);
}

// =========================================================================
// Succession helpers (requirement 3)
// =========================================================================

BTNode<FamilyMember>* MafiaTree::findFirstAvailable(BTNode<FamilyMember>* node) const {
    if (!node) return nullptr;
    if (!node->data.is_dead && !node->data.in_jail) return node;

    BTNode<FamilyMember>* leftResult = findFirstAvailable(node->left);
    if (leftResult) return leftResult;

    return findFirstAvailable(node->right);
}

BTNode<FamilyMember>* MafiaTree::findNearestBossWithTwoSuccessors(BTNode<FamilyMember>* bossNode) const {
    // Walk up through parents, looking for an ancestor that has both left and right
    // children, with at least one alive and free.
    BTNode<FamilyMember>* current = getParentOf(bossNode);
    while (current) {
        if (current->left && current->right) {
            // Check if at least one side has a viable candidate
            BTNode<FamilyMember>* candidate = findFirstAvailable(current->left);
            if (!candidate) candidate = findFirstAvailable(current->right);
            if (candidate) return current;
        }
        current = getParentOf(current);
    }
    return nullptr;
}

// =========================================================================
// Succession promotion (requirement 3)
// =========================================================================

void MafiaTree::evaluateSuccession() {
    BTNode<FamilyMember>* bossNode = getBossNode();
    if (!bossNode) return;

    FamilyMember& boss = bossNode->data;

    // Check if succession must trigger:
    //  - boss is dead, OR
    //  - boss is in jail, OR
    //  - boss is 70+ years old
    bool mustReplace = boss.is_dead || boss.in_jail || boss.age >= 70;
    if (!mustReplace) return;

    BTNode<FamilyMember>* newBossNode = nullptr;

    // Rule 3.1 — Boss has successors: pick first alive+free in its own sub-tree (left branch first).
    if (bossNode->left || bossNode->right) {
        newBossNode = findFirstAvailable(bossNode->left);
        if (!newBossNode) newBossNode = findFirstAvailable(bossNode->right);
    }

    // Rule 3.2/3.3 — No successors or none available: go to partner sub-tree from boss's parent.
    if (!newBossNode) {
        BTNode<FamilyMember>* grandParent = getParentOf(bossNode);
        if (grandParent) {
            // The "partner" of bossNode is the other child of grandParent.
            BTNode<FamilyMember>* partner = (grandParent->left == bossNode)
                                              ? grandParent->right
                                              : grandParent->left;
            if (partner) {
                // Rule 3.3: if partner is alive+free and has no successors, partner becomes boss.
                if (!partner->data.is_dead && !partner->data.in_jail
                    && !partner->left && !partner->right) {
                    newBossNode = partner;
                } else {
                    // Otherwise: first available in partner's sub-tree.
                    newBossNode = findFirstAvailable(partner->left);
                    if (!newBossNode) newBossNode = findFirstAvailable(partner->right);
                    // Rule 3.3 again: partner itself if available and no successors already checked.
                    if (!newBossNode && !partner->data.is_dead && !partner->data.in_jail) {
                        newBossNode = partner;
                    }
                }
            }
        }
    }

    // Rule 3.4 — Search in boss's grandparent's other branch.
    if (!newBossNode) {
        BTNode<FamilyMember>* grandParent = getParentOf(bossNode);
        BTNode<FamilyMember>* greatGrandParent = grandParent ? getParentOf(grandParent) : nullptr;
        if (greatGrandParent) {
            BTNode<FamilyMember>* uncle = (greatGrandParent->left == grandParent)
                                            ? greatGrandParent->right
                                            : greatGrandParent->left;
            if (uncle) {
                newBossNode = findFirstAvailable(uncle->left);
                if (!newBossNode) newBossNode = findFirstAvailable(uncle->right);
                if (!newBossNode && !uncle->data.is_dead && !uncle->data.in_jail) {
                    newBossNode = uncle;
                }
            }
        }
    }

    // Rule 3.5 — Find nearest ancestor with two successors, pick from there.
    if (!newBossNode) {
        BTNode<FamilyMember>* ancestor = findNearestBossWithTwoSuccessors(bossNode);
        if (ancestor) {
            newBossNode = findFirstAvailable(ancestor->left);
            if (!newBossNode) newBossNode = findFirstAvailable(ancestor->right);
        }
    }

    // Rule 3.6 — All free candidates exhausted; try among jailed members.
    if (!newBossNode) {
        // Re-run the same logic but ignoring the in_jail filter.
        struct JailSearch {
            static BTNode<FamilyMember>* find(BTNode<FamilyMember>* n) {
                if (!n) return nullptr;
                if (!n->data.is_dead) return n; // alive, even if in jail
                BTNode<FamilyMember>* l = find(n->left);
                if (l) return l;
                return find(n->right);
            }
        };
        newBossNode = JailSearch::find(root);
    }

    if (!newBossNode || newBossNode == bossNode) return; // nobody to promote

    // Demote old boss
    boss.is_boss = false;
    if (!boss.is_dead) boss.was_boss = true;

    // Promote new boss
    newBossNode->data.is_boss = true;
    newBossNode->data.was_boss = false;

    // Rebuild parent map since tree topology did not change, but data did.
    // (No topology change needed — we only updated data fields.)
}

// =========================================================================
// Modification (requirement 4)
// =========================================================================

bool MafiaTree::updateMember(int id,
                             const std::string& name,
                             const std::string& last_name,
                             char gender,
                             int age,
                             bool is_dead,
                             bool in_jail) {
    FamilyMember* member = findMemberById(id);
    if (!member) return false;

    member->name      = name;
    member->last_name = last_name;
    member->gender    = gender;
    member->age       = age;
    member->is_dead   = is_dead;
    member->in_jail   = in_jail;

    // Age-out or jail rule: if the updated member is the boss, re-evaluate.
    if (member->is_boss) {
        evaluateSuccession();
    }

    return true;
}

// =========================================================================
// Serialization
// =========================================================================

void MafiaTree::toList(cde::LinkedList<FamilyMember>& out) const {
    // Pre-order traversal
    struct Serializer {
        static void collect(BTNode<FamilyMember>* n, cde::LinkedList<FamilyMember>& list) {
            if (!n) return;
            list.push_back(n->data);
            collect(n->left, list);
            collect(n->right, list);
        }
    };
    Serializer::collect(root, out);
}
