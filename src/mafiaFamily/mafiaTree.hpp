#ifndef MAFIA_TREE_HPP
#define MAFIA_TREE_HPP

#include "../csvReader/csvReader.hpp"
#include "../external/BinaryTree/binaryTree.hpp"

/**
 * @brief A Binary Tree specialized for the Mafia Family hierarchy.
 *
 * Each node can have at most TWO children:
 *  - left  = first successor (primary heir)
 *  - right = second successor (backup heir)
 *
 * The tree is built from a LinkedList<FamilyMember> produced by CSVReader.
 * id_boss == 0 identifies the root (the current/original boss).
 */
class MafiaTree : public BinaryTree<FamilyMember> {
private:
  /**
   * @brief Recursively searches for a node whose member has the given id.
   * @return Pointer to the BTNode, or nullptr if not found.
   */
  BTNode<FamilyMember> *findById(BTNode<FamilyMember> *node, int id) const;

  /**
   * @brief Inserts a FamilyMember as a child of the node with id ==
   * member.id_boss. Left child = first successor, right child = second
   * successor.
   * @return true if the member was successfully inserted.
   */
  bool insertUnder(BTNode<FamilyMember> *node, const FamilyMember &member);

  // ----------------------------------------------------------------
  // Succession helpers
  // ----------------------------------------------------------------

  /**
   * @brief Finds the first available successor (alive, not in jail) using
   * pre-order traversal.
   * @param node  Root of the sub-tree to search.
   * @return Pointer to the qualifying BTNode, or nullptr if none found.
   */
   BTNode<FamilyMember> *findFirstAvailable(BTNode<FamilyMember> *node, bool ignoreJail = false) const;

  /**
   * @brief Finds the nearest ancestor of `start` that has two successors,
   *        at least one alive and outside of jail.
   * @param start The node from which to begin climbing the tree.
   * @return Pointer to the qualifying BTNode, or nullptr.
   */
  BTNode<FamilyMember> *
  findNearestBossWithTwoSuccessors(BTNode<FamilyMember> *bossNode, bool ignoreJail = false) const;

  /**
   * @brief Builds an auxiliary parent-lookup structure so that we can
   *        navigate upward while resolving succession.
   *        Stores pairs (child_ptr -> parent_ptr) in a linked list style
   */
  struct ParentEntry {
    BTNode<FamilyMember> *node;
    BTNode<FamilyMember> *parent;
    ParentEntry *next;
  };

  // Linked-list of ParentEntry (no vectors allowed)
  ParentEntry *parentMap = nullptr;

  void buildParentMap(BTNode<FamilyMember> *node, BTNode<FamilyMember> *parent);
  void clearParentMap();
  BTNode<FamilyMember> *getParentOf(BTNode<FamilyMember> *node) const;

public:
  MafiaTree();
  ~MafiaTree() override;

  // ----------------------------------------------------------------
  // Construction
  // ----------------------------------------------------------------

  /**
   * @brief Builds the family tree from a LinkedList of FamilyMembers.
   *        The member with id_boss == 0 becomes the root.
   *        Must be called before any other operation.
   * @param members  Pointer to the list produced by CSVReader::load().
   *                 Ownership is NOT transferred.
   */
  void buildFromList(const cde::LinkedList<FamilyMember> *members);

  // ----------------------------------------------------------------
  // Queries
  // ----------------------------------------------------------------

  /**
   * @brief Returns a pointer to the data of the node with the given id.
   *        Can be used to read or modify a member (except id / id_boss).
   */
  FamilyMember *findMemberById(int id);

  /**
   * @brief Returns a pointer to the current boss node.
   */
  BTNode<FamilyMember> *getBossNode() const;

  /**
   * @brief Collects the succession line (alive, not in jail) in order.
   *        Results are appended to `out` — a caller-managed LinkedList.
   * @param out  LinkedList<FamilyMember> to fill with the succession order.
   */
  void getSuccessionLine(cde::LinkedList<FamilyMember> &out) const;

  // ----------------------------------------------------------------
  // Succession logic — requirement 3
  // ----------------------------------------------------------------

  /**
   * @brief Evaluates whether the current boss must be replaced and, if so,
   *        applies all succession rules to find and promote a new boss.
   *        Called automatically after any status change to the boss.
   */
  void evaluateSuccession();

  // ----------------------------------------------------------------
  // Modification — requirement 4
  // ----------------------------------------------------------------

  /**
   * @brief Updates all editable fields of a member (everything except id and
   * id_boss).
   * @param id             ID of the member to update.
   * @param name           New name.
   * @param last_name      New last name.
   * @param gender         New gender ('H' / 'M').
   * @param age            New age.
   * @param is_dead        New dead status.
   * @param in_jail        New jail status.
   * @return true if the member was found and updated.
   */
  bool updateMember(int id, const std::string &name,
                    const std::string &last_name, char gender, int age,
                    bool is_dead, bool in_jail);

  /**
   * @brief Serializes the tree back into a LinkedList for saving via
   * CSVReader::save().
   * @param out  LinkedList to fill (in pre-order traversal).
   */
  void toList(cde::LinkedList<FamilyMember> &out) const;

  /**
   * @brief Generates a list of strings representing the tree structure with
   * indentation.
   * @param out LinkedList to fill with formatted strings.
   */
  void toIndentedList(cde::LinkedList<std::string> &out) const;
};

#endif // MAFIA_TREE_HPP
