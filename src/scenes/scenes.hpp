#ifndef SCENES_HPP
#define SCENES_HPP

#include "../csvReader/csvReader.hpp"
#include "../external/GuayabaEngine/GuayabaConsoleEngine.hpp"
#include "../mafiaFamily/mafiaTree.hpp"
#include <sstream>
#include <string>

// Shared application state accessible by all scenes
struct AppState {
  MafiaTree tree;
  std::string csvPath;
  bool dataLoaded = false;
};

// Forward declarations of scene-builder functions
std::shared_ptr<Node> createMainMenu(AppState &state);
std::shared_ptr<Node> createSuccessionScene(AppState &state);
std::shared_ptr<Node> createTreeViewScene(AppState &state);
std::shared_ptr<Node> createModifyScene(AppState &state);

// Helper: render a centered title banner
inline std::shared_ptr<NodePCT> makeBanner(const std::string &title) {
  // Frame: 78 chars wide
  std::string border(78, '=');
  int pad = (76 - (int)title.size()) / 2;
  std::string middle = "|" + std::string(pad, ' ') + title +
                       std::string(76 - pad - (int)title.size(), ' ') + "|";
  return std::make_shared<NodePCT>(
      "Banner", Vec2{1, 1}, "BRIGHT_YELLOW", "BLACK",
      std::vector<std::string>{border, middle, border});
}

// Scene 1 — Main Menu
inline std::shared_ptr<Node> createMainMenu(AppState &state) {
  auto root = std::make_shared<Node>("MainMenu");

  // ── Banner ──────────────────────────────────────────────────────────
  root->addChild(makeBanner("LA FAMILIA - Sistema de Sucesion"));

  // ── Status line ─────────────────────────────────────────────────────
  auto statusLabel = std::make_shared<NodePCT>(
      "StatusLabel", Vec2{2, 6}, "CYAN", "BLACK",
      std::vector<std::string>{"Estado: " +
                               (state.dataLoaded
                                    ? "Datos cargados desde " + state.csvPath
                                    : "Sin datos cargados")});
  root->addChild(statusLabel);

  // ── Menu buttons ─────────────────────────────────────────────────────
  auto btn1 = std::make_shared<NodeButton>(
      "BtnSuccession", Vec2{5, 10}, "BLACK", "BRIGHT_GREEN",
      std::vector<std::string>{"  [1] Ver linea de sucesion          "});

  auto btn2 = std::make_shared<NodeButton>(
      "BtnTreeView", Vec2{5, 12}, "BLACK", "BRIGHT_MAGENTA",
      std::vector<std::string>{"  [2] Ver arbol completo (jerarquia) "});

  auto btn3 = std::make_shared<NodeButton>(
      "BtnModify", Vec2{5, 14}, "BLACK", "BRIGHT_CYAN",
      std::vector<std::string>{"  [3] Modificar miembro de la familia"});

  auto btn4 = std::make_shared<NodeButton>(
      "BtnSave", Vec2{5, 16}, "BLACK", "BRIGHT_YELLOW",
      std::vector<std::string>{"  [4] Guardar cambios al CSV         "});

  auto btn5 = std::make_shared<NodeButton>(
      "BtnExit", Vec2{5, 18}, "BLACK", "BRIGHT_RED",
      std::vector<std::string>{"  [5] Salir                          "});

  // ── Button actions ───────────────────────────────────────────────────
  btn1->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createSuccessionScene(state));
  });

  btn2->setOnClick([&state]() {
    if (!state.dataLoaded) return;
    SceneManager::getInstance().changeScene(createTreeViewScene(state));
  });

  btn3->setOnClick([&state]() {
    if (!state.dataLoaded)
      return;
    SceneManager::getInstance().changeScene(createModifyScene(state));
  });

  btn4->setOnClick([&state, statusLabel]() {
    if (!state.dataLoaded)
      return;
    cde::LinkedList<FamilyMember> list;
    state.tree.toList(list);
    CSVReader::save(state.csvPath, list);
    statusLabel->set_text({"Estado: Cambios guardados en " + state.csvPath});
    statusLabel->changeTextColor("BRIGHT_GREEN");
  });

  btn5->setOnClick([]() { SceneManager::getInstance().stopRunning(); });

  root->addChild(btn1);
  root->addChild(btn2);
  root->addChild(btn3);
  root->addChild(btn4);
  root->addChild(btn5);

  // ── Keyboard shortcut handling via process function ───────────────────
  root->setProcessFunction([&state, statusLabel](double) {
    if (Input::lastChar == '1') {
      SceneManager::getInstance().changeScene(createSuccessionScene(state));
    } else if (Input::lastChar == '2') {
      if (state.dataLoaded)
        SceneManager::getInstance().changeScene(createTreeViewScene(state));
    } else if (Input::lastChar == '3') {
      if (state.dataLoaded)
        SceneManager::getInstance().changeScene(createModifyScene(state));
    } else if (Input::lastChar == '4') {
      if (state.dataLoaded) {
        cde::LinkedList<FamilyMember> list;
        state.tree.toList(list);
        CSVReader::save(state.csvPath, list);
        statusLabel->set_text(
            {"Estado: Cambios guardados en " + state.csvPath});
        statusLabel->changeTextColor("BRIGHT_GREEN");
      }
    } else if (Input::lastChar == '5') {
      SceneManager::getInstance().stopRunning();
    }
  });

  auto hint = std::make_shared<NodePCT>(
      "Hint", Vec2{2, 57}, "WHITE", "BLACK",
      std::vector<std::string>{
          "Usa click del raton o los botones numerados [1-5] para navegar."});
  root->addChild(hint);

  return root;
}

// Scene 2 — Succession Line
inline std::shared_ptr<Node> createSuccessionScene(AppState &state) {
  auto root = std::make_shared<Node>("SuccessionScene");

  root->addChild(makeBanner("Linea de Sucesion Actual"));

  if (!state.dataLoaded) {
    root->addChild(std::make_shared<NodePCT>(
        "NoData", Vec2{2, 6}, "BRIGHT_RED", "BLACK",
        std::vector<std::string>{"  No hay datos cargados."}));
  } else {
    cde::LinkedList<FamilyMember> succession;
    state.tree.getSuccessionLine(succession);

    int yPos = 6;
    int rank = 1;

    // Header row
    root->addChild(std::make_shared<NodePCT>(
        "Header", Vec2{2, yPos}, "BRIGHT_WHITE", "BLACK",
        std::vector<std::string>{"  # | ID  | Nombre              | Apellido   "
                                 "         | Genero | Edad | Jefe"}));
    yPos++;
    root->addChild(std::make_shared<NodePCT>(
        "Sep", Vec2{2, yPos}, "WHITE", "BLACK",
        std::vector<std::string>{std::string(76, '-')}));
    yPos++;

    int sz = succession.get_size();
    for (int i = 0; i < sz && yPos < 54; ++i) {
      FamilyMember m = succession.get(i);

      // Build a formatted row
      auto pad = [](const std::string &s, int w) {
        return s.size() >= (size_t)w ? s.substr(0, w)
                                     : s + std::string(w - s.size(), ' ');
      };
      auto ipad = [&pad](int n, int w) {
        std::stringstream ss;
        ss << n;
        return pad(ss.str(), w);
      };

      std::string row = "  " + ipad(rank, 2) + "| " + ipad(m.id, 3) + " | " +
                        pad(m.name, 19) + " | " + pad(m.last_name, 19) + " | " +
                        std::string(1, m.gender) + "      | " + ipad(m.age, 4) +
                        " | " + (m.is_boss ? "SI" : "  ");

      std::string color = m.is_boss ? "BRIGHT_YELLOW" : "BRIGHT_WHITE";
      root->addChild(std::make_shared<NodePCT>("Row" + std::to_string(rank),
                                               Vec2{2, yPos}, color, "BLACK",
                                               std::vector<std::string>{row}));

      ++rank;
      ++yPos;
    }

    if (sz == 0) {
      root->addChild(std::make_shared<NodePCT>(
          "Empty", Vec2{2, yPos}, "BRIGHT_RED", "BLACK",
          std::vector<std::string>{"  No hay miembros vivos y libres."}));
    }
  }

  auto btnBack = std::make_shared<NodeButton>(
      "BtnBack", Vec2{60, 4}, "BLACK", "BRIGHT_RED",
      std::vector<std::string>{"  [M] Volver al menu  "});
  btnBack->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createMainMenu(state));
  });
  root->addChild(btnBack);

  // ── Back shortcut ───────────────────────────────────────────────────────
  root->setProcessFunction([&state](double) {
    if (Input::lastChar == 'm' || Input::lastChar == 'M') {
      SceneManager::getInstance().changeScene(createMainMenu(state));
    }
  });

  return root;
}

// Scene 2.5 — Indented Tree View
inline std::shared_ptr<Node> createTreeViewScene(AppState &state) {
  auto root = std::make_shared<Node>("TreeViewScene");

  root->addChild(makeBanner("Jerarquia Familiar Completa"));

  if (!state.dataLoaded) {
    root->addChild(std::make_shared<NodePCT>(
        "NoData", Vec2{2, 6}, "BRIGHT_RED", "BLACK",
        std::vector<std::string>{"  No hay datos cargados."}));
  } else {
    cde::LinkedList<std::string> indentedTree;
    state.tree.toIndentedList(indentedTree);

    int yPos = 6;
    int sz = indentedTree.get_size();
    for (int i = 0; i < sz && yPos < 54; ++i) {
      std::string line = indentedTree.get(i);
      
      std::string color = "BRIGHT_WHITE";
      if (line.find("[BOSS]") != std::string::npos) color = "BRIGHT_YELLOW";
      else if (line.find("[MUERTO]") != std::string::npos) color = "BRIGHT_RED";
      else if (line.find("[CARCEL]") != std::string::npos) color = "RED";

      root->addChild(std::make_shared<NodePCT>("TreeRow" + std::to_string(i),
                                               Vec2{2, yPos}, color, "BLACK",
                                               std::vector<std::string>{line}));
      ++yPos;
    }

    if (sz == 0) {
      root->addChild(std::make_shared<NodePCT>(
          "Empty", Vec2{2, yPos}, "BRIGHT_RED", "BLACK",
          std::vector<std::string>{"  El arbol esta vacio."}));
    }
  }

  auto btnBack = std::make_shared<NodeButton>(
      "BtnBack", Vec2{60, 4}, "BLACK", "BRIGHT_RED",
      std::vector<std::string>{"  [M] Volver al menu  "});
  btnBack->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createMainMenu(state));
  });
  root->addChild(btnBack);

  // ── Back shortcut ───────────────────────────────────────────────────────
  root->setProcessFunction([&state](double) {
    if (Input::lastChar == 'm' || Input::lastChar == 'M') {
      SceneManager::getInstance().changeScene(createMainMenu(state));
    }
  });

  return root;
}

// Scene 3 — Modify Member
inline std::shared_ptr<Node> createModifyScene(AppState &state) {
  auto root = std::make_shared<Node>("ModifyScene");

  root->addChild(makeBanner("Modificar Miembro de la Familia"));

  // State to track if we've found the member and built the UI
  struct ModState {
    int memberId = -1;
    bool found = false;
    
    int framesWait = 0;

    // Temp values for edits
    std::string name;
    std::string lastName;
    char gender;
    int age;
    bool isDead;
    bool inJail;
  };
  auto mS = std::make_shared<ModState>();

  auto prompt = std::make_shared<NodePCT>(
      "Prompt", Vec2{2, 6}, "BRIGHT_WHITE", "BLACK",
      std::vector<std::string>{"  Ingresa el ID del miembro a modificar:"});
  root->addChild(prompt);

  auto debugLabel = std::make_shared<NodePCT>(
      "DebugLabel", Vec2{2, 7}, "BRIGHT_YELLOW", "BLACK",
      std::vector<std::string>{"Status: Esperando ID..."});
  root->addChild(debugLabel);

  auto btnBack = std::make_shared<NodeButton>(
      "BtnBack", Vec2{60, 4}, "BLACK", "BRIGHT_RED",
      std::vector<std::string>{"  [M] Volver al menu  "});
  btnBack->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createMainMenu(state));
  });
  root->addChild(btnBack);

  auto logicNode = std::make_shared<Node>("ModifyLogic");
  
  logicNode->setProcessFunction([&state, mS, root, debugLabel, prompt](double) {
    if (mS->found) return;

    // Wait 2 frames to ensure the banner and prompt are rendered
    if (mS->framesWait < 2) { mS->framesWait++; return; }

    // Sequential blocking input for ID Search (remains text)
    std::string idStr = Input::getLineInput({44, 6});
    if (idStr.empty()) return;

    try { mS->memberId = std::stoi(idStr); } catch(...) { mS->memberId = -1; }

    FamilyMember* member = state.tree.findMemberById(mS->memberId);
    if (!member) {
      debugLabel->set_text({"Status: ERROR - ID " + std::to_string(mS->memberId) + " no encontrado!"});
      debugLabel->changeTextColor("BRIGHT_RED");
      mS->framesWait = 0; 
      return;
    }

    // Found! Transition to interactive UI
    mS->found = true;
    mS->name = member->name;
    mS->lastName = member->last_name;
    mS->gender = member->gender;
    mS->age = member->age;
    mS->isDead = member->is_dead;
    mS->inJail = member->in_jail;

    prompt->set_text({"  Editando Miembro (ID: " + std::to_string(mS->memberId) + "): " + mS->name + " " + mS->lastName});
    prompt->changeTextColor("BRIGHT_GREEN");
    debugLabel->set_text({"Status: Miembro encontrado. Haz clic en los botones para modificar."});
    debugLabel->changeTextColor("BRIGHT_CYAN");

    // Clear ID input area visually
    root->addChild(std::make_shared<NodeBox>("ClearID", Vec2{44, 6}, Vec2{20, 1}, "BLACK"));

    // --- Build editing UI ---
    int y = 10;
    auto addLabel = [&](const std::string& txt, int yPos) {
        root->addChild(std::make_shared<NodePCT>("L_"+txt, Vec2{2, yPos}, "BRIGHT_WHITE", "BLACK", std::vector<std::string>{txt + ":"}));
    };

    // 1. Name (Text Input)
    addLabel("Nombre   ", y);
    auto btnName = std::make_shared<NodeButton>("btnName", Vec2{15, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{" " + mS->name + " "});
    btnName->setOnClick([mS, btnName, debugLabel]() {
        debugLabel->set_text({"Status: Escribe el nuevo NOMBRE y pulsa ENTER..."});
        debugLabel->changeTextColor("BRIGHT_YELLOW");
        std::string res = Input::getLineInput(btnName->getGlobalPosition() + Vec2{1, 0});
        if (!res.empty()) { mS->name = res; btnName->set_text({" " + mS->name + " "}); }
        debugLabel->set_text({"Status: Nombre actualizado localmente."});
        debugLabel->changeTextColor("BRIGHT_CYAN");
    });
    root->addChild(btnName);
    y += 2;

    // 2. Last Name (Text Input)
    addLabel("Apellido ", y);
    auto btnLast = std::make_shared<NodeButton>("btnLast", Vec2{15, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{" " + mS->lastName + " "});
    btnLast->setOnClick([mS, btnLast, debugLabel]() {
        debugLabel->set_text({"Status: Escribe el nuevo APELLIDO y pulsa ENTER..."});
        debugLabel->changeTextColor("BRIGHT_YELLOW");
        std::string res = Input::getLineInput(btnLast->getGlobalPosition() + Vec2{1, 0});
        if (!res.empty()) { mS->lastName = res; btnLast->set_text({" " + mS->lastName + " "}); }
        debugLabel->set_text({"Status: Apellido actualizado localmente."});
        debugLabel->changeTextColor("BRIGHT_CYAN");
    });
    root->addChild(btnLast);
    y += 2;

    // 3. Gender (Selection Buttons - Fixed value)
    addLabel("Genero   ", y);
    auto btnH = std::make_shared<NodeButton>("btnH", Vec2{15, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{" [ Hombre (H) ] "});
    auto btnM = std::make_shared<NodeButton>("btnM", Vec2{31, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{" [ Mujer (M) ] "});
    auto updateG = [mS, btnH, btnM]() {
        btnH->changeBackgroundColor(mS->gender == 'H' ? "BRIGHT_GREEN" : "BRIGHT_WHITE");
        btnM->changeBackgroundColor(mS->gender == 'M' ? "BRIGHT_GREEN" : "BRIGHT_WHITE");
    };
    btnH->setOnClick([mS, updateG]() { mS->gender = 'H'; updateG(); });
    btnM->setOnClick([mS, updateG]() { mS->gender = 'M'; updateG(); });
    root->addChild(btnH);
    root->addChild(btnM);
    updateG();
    y += 2;

    // 4. Age (Text Input)
    addLabel("Edad     ", y);
    auto btnAge = std::make_shared<NodeButton>("btnAge", Vec2{15, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{" " + std::to_string(mS->age) + " "});
    btnAge->setOnClick([mS, btnAge, debugLabel]() {
        debugLabel->set_text({"Status: Escribe la nueva EDAD y pulsa ENTER..."});
        debugLabel->changeTextColor("BRIGHT_YELLOW");
        std::string res = Input::getLineInput(btnAge->getGlobalPosition() + Vec2{1, 0});
        if (!res.empty()) {
            try { mS->age = std::stoi(res); btnAge->set_text({" " + std::to_string(mS->age) + " "}); } catch(...) {}
        }
        debugLabel->set_text({"Status: Edad actualizada localmente."});
        debugLabel->changeTextColor("BRIGHT_CYAN");
    });
    root->addChild(btnAge);
    y += 2;

    // 5. Is Dead (Selection Buttons - Fixed value)
    addLabel("Muerto?  ", y);
    auto btnDeadSi = std::make_shared<NodeButton>("btnDSi", Vec2{15, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{"   SI   "});
    auto btnDeadNo = std::make_shared<NodeButton>("btnDNo", Vec2{25, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{"   NO   "});
    auto updateD = [mS, btnDeadSi, btnDeadNo]() {
        btnDeadSi->changeBackgroundColor(mS->isDead ? "BRIGHT_RED" : "BRIGHT_WHITE");
        btnDeadNo->changeBackgroundColor(!mS->isDead ? "BRIGHT_GREEN" : "BRIGHT_WHITE");
    };
    btnDeadSi->setOnClick([mS, updateD]() { mS->isDead = true; updateD(); });
    btnDeadNo->setOnClick([mS, updateD]() { mS->isDead = false; updateD(); });
    root->addChild(btnDeadSi);
    root->addChild(btnDeadNo);
    updateD();
    y += 2;

    // 6. In Jail (Selection Buttons - Fixed value)
    addLabel("Carcel?  ", y);
    auto btnJailSi = std::make_shared<NodeButton>("btnJSi", Vec2{15, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{"   SI   "});
    auto btnJailNo = std::make_shared<NodeButton>("btnJNo", Vec2{25, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{"   NO   "});
    auto updateJ = [mS, btnJailSi, btnJailNo]() {
        btnJailSi->changeBackgroundColor(mS->inJail ? "BRIGHT_RED" : "BRIGHT_WHITE");
        btnJailNo->changeBackgroundColor(!mS->inJail ? "BRIGHT_GREEN" : "BRIGHT_WHITE");
    };
    btnJailSi->setOnClick([mS, updateJ]() { mS->inJail = true; updateJ(); });
    btnJailNo->setOnClick([mS, updateJ]() { mS->inJail = false; updateJ(); });
    root->addChild(btnJailSi);
    root->addChild(btnJailNo);
    updateJ();
    y += 4;

    // --- Footer Controls ---
    auto btnSave = std::make_shared<NodeButton>("btnSave", Vec2{5, y}, "BLACK", "BRIGHT_YELLOW", std::vector<std::string>{"        [ GUARDAR CAMBIOS Y VOLVER ]        "});
    btnSave->setOnClick([&state, mS]() {
        state.tree.updateMember(mS->memberId, mS->name, mS->lastName, mS->gender, mS->age, mS->isDead, mS->inJail);
        SceneManager::getInstance().changeScene(createMainMenu(state));
    });
    root->addChild(btnSave);

    auto btnCancel = std::make_shared<NodeButton>("btnCancel", Vec2{50, y}, "BLACK", "BRIGHT_WHITE", std::vector<std::string>{"   [ CANCELAR ]   "});
    btnCancel->setOnClick([&state]() {
        SceneManager::getInstance().changeScene(createMainMenu(state));
    });
    root->addChild(btnCancel);
  });
  root->addChild(logicNode);

  // Main menu shortcut
  root->setProcessFunction([&state](double) {
    if (Input::lastChar == 'm' || Input::lastChar == 'M') {
      SceneManager::getInstance().changeScene(createMainMenu(state));
    }
  });

  return root;
}

#endif // SCENES_HPP
