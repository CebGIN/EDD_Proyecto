#include "csvReader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

std::string FamilyMember::to_csv_line() const {
    std::stringstream ss;
    ss << id << "," 
       << name << "," 
       << last_name << "," 
       << gender << "," 
       << age << "," 
       << id_boss << "," 
       << (is_dead ? 1 : 0) << "," 
       << (in_jail ? 1 : 0) << "," 
       << (was_boss ? 1 : 0) << "," 
       << (is_boss ? 1 : 0);
    return ss.str();
}

cde::LinkedList<FamilyMember>* CSVReader::load(const std::string& filename) {
    auto* list = new cde::LinkedList<FamilyMember>();
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return list;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Skip the header line if it looks like one
        if (isHeader && line.find("id") != std::string::npos) {
            isHeader = false;
            continue;
        }
        isHeader = false;

        try {
            list->push_back(parse_line(line));
        } catch (...) {
            // Skip malformed lines
        }
    }

    file.close();
    return list;
}

void CSVReader::save(const std::string& filename, const cde::LinkedList<FamilyMember>& members) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    // Write Header
    file << "id,name,last_name,gender,age,id_boss,is_dead,in_jail,was_boss,is_boss\n";

    int size = members.get_size();
    for (int i = 0; i < size; ++i) {
        file << members.get(i).to_csv_line() << "\n";
    }

    file.close();
}

FamilyMember CSVReader::parse_line(const std::string& line) {
    std::stringstream ss(line);
    std::string item;
    FamilyMember member;

    auto get_next = [&]() {
        std::string val;
        if (std::getline(ss, val, ',')) {
            return trim(val);
        }
        return std::string("");
    };

    member.id = std::stoi(get_next());
    member.name = get_next();
    member.last_name = get_next();
    
    std::string genderStr = get_next();
    member.gender = genderStr.empty() ? ' ' : genderStr[0];
    
    member.age = std::stoi(get_next());
    member.id_boss = std::stoi(get_next());
    member.is_dead = (get_next() == "1");
    member.in_jail = (get_next() == "1");
    member.was_boss = (get_next() == "1");
    member.is_boss = (get_next() == "1");

    return member;
}

std::string CSVReader::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
