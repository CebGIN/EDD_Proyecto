#include "./external/GuayabaEngine/GuayabaConsoleEngine.hpp"

std::shared_ptr<Node> createMainMenu(){
    return std::make_shared<Node>("Main");
}

int main (){
    SceneManager::getInstance().changeScene(createMainMenu());
    
}