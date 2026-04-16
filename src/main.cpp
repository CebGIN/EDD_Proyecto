#include "./external/GuayabaEngine/GuayabaConsoleEngine.hpp"
#include "./scenes/scenes.hpp"
#include "./csvReader/csvReader.hpp"

int main() {
    // ------------------------------------------------------------------
    // Bootstrap — load data and build the tree
    // ------------------------------------------------------------------
    AppState state;
    state.csvPath   = "bin/test_data.csv";
    state.dataLoaded = false;

    cde::LinkedList<FamilyMember>* members = CSVReader::load(state.csvPath);
    if (members && members->get_size() > 0) {
        state.tree.buildFromList(members);
        state.dataLoaded = true;
    }
    delete members;

    // ------------------------------------------------------------------
    // Launch UI
    // ------------------------------------------------------------------
    SceneManager::getInstance().changeScene(createMainMenu(state));
    SceneManager::getInstance().startRunning();

    return 0;
}