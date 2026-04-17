#include "./csvReader/csvReader.hpp"
#include "./external/GuayabaEngine/GuayabaConsoleEngine.hpp"
#include "./scenes/scenes.hpp"

int main() {
  AppState state;
  state.csvPath = "bin/test_data.csv";
  state.dataLoaded = false;

  cde::LinkedList<FamilyMember> *members = CSVReader::load(state.csvPath);
  if (members && members->get_size() > 0) {
    state.tree.buildFromList(members);
    state.dataLoaded = true;
  }
  delete members;

  SceneManager::getInstance().changeScene(createMainMenu(state));
  SceneManager::getInstance().startRunning();

  return 0;
}