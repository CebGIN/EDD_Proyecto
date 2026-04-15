#ifndef CSV_READER_HPP
#define CSV_READER_HPP

#include <string>
#include "../external/CGINDE.hpp"

/**
 * @brief Represents a member of the mafia family as defined in the project requirements.
 */
struct FamilyMember {
    int id;
    std::string name;
    std::string last_name;
    char gender; // 'H' or 'M'
    int age;
    int id_boss;
    bool is_dead;
    bool in_jail;
    bool was_boss;
    bool is_boss;

    /**
     * @brief Converts the structure data into a comma-separated string.
     * @return A string formatted for CSV output.
     */
    std::string to_csv_line() const;
    
    /**
     * @brief Equality operator for comparison (useful for LinkedList operations).
     */
    bool operator==(const FamilyMember& other) const {
        return id == other.id;
    }
};

/**
 * @brief A utility class to read and write FamilyMember data to/from CSV files.
 */
class CSVReader {
public:
    /**
     * @brief Loads all family members from a CSV file into a LinkedList.
     * @param filename Path to the CSV file.
     * @return A pointer to a new LinkedList of FamilyMembers. The caller is responsible for deleting it.
     */
    static cde::LinkedList<FamilyMember>* load(const std::string& filename);

    /**
     * @brief Saves a LinkedList of family members to a CSV file.
     * @param filename Path to the output CSV file.
     * @param members The list of members to save.
     */
    static void save(const std::string& filename, const cde::LinkedList<FamilyMember>& members);
    
private:
    /**
     * @brief Parses a single line from a CSV file into a FamilyMember struct.
     * @param line The raw CSV line.
     * @return A populated FamilyMember struct.
     */
    static FamilyMember parse_line(const std::string& line);
    
    /**
     * @brief Trims whitespace from both ends of a string.
     */
    static std::string trim(const std::string& str);
};

#endif // CSV_READER_HPP
