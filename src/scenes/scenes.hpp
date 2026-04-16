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
      "BtnModify", Vec2{5, 12}, "BLACK", "BRIGHT_CYAN",
      std::vector<std::string>{"  [2] Modificar miembro de la familia"});

  auto btn3 = std::make_shared<NodeButton>(
      "BtnSave", Vec2{5, 14}, "BLACK", "BRIGHT_YELLOW",
      std::vector<std::string>{"  [3] Guardar cambios al CSV         "});

  auto btn4 = std::make_shared<NodeButton>(
      "BtnExit", Vec2{5, 16}, "BLACK", "BRIGHT_RED",
      std::vector<std::string>{"  [4] Salir                          "});

  // ── Button actions ───────────────────────────────────────────────────
  btn1->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createSuccessionScene(state));
  });

  btn2->setOnClick([&state]() {
    if (!state.dataLoaded)
      return;
    SceneManager::getInstance().changeScene(createModifyScene(state));
  });

  btn3->setOnClick([&state, statusLabel]() {
    if (!state.dataLoaded)
      return;
    cde::LinkedList<FamilyMember> list;
    state.tree.toList(list);
    CSVReader::save(state.csvPath, list);
    statusLabel->set_text({"Estado: Cambios guardados en " + state.csvPath});
    statusLabel->changeTextColor("BRIGHT_GREEN");
  });

  btn4->setOnClick([]() { SceneManager::getInstance().stopRunning(); });

  root->addChild(btn1);
  root->addChild(btn2);
  root->addChild(btn3);
  root->addChild(btn4);

  // ── Keyboard shortcut handling via process function ───────────────────
  root->setProcessFunction([&state, statusLabel](double) {
    if (Input::lastChar == '1') {
      SceneManager::getInstance().changeScene(createSuccessionScene(state));
    } else if (Input::lastChar == '2') {
      if (state.dataLoaded)
        SceneManager::getInstance().changeScene(createModifyScene(state));
    } else if (Input::lastChar == '3') {
      if (state.dataLoaded) {
        cde::LinkedList<FamilyMember> list;
        state.tree.toList(list);
        CSVReader::save(state.csvPath, list);
        statusLabel->set_text(
            {"Estado: Cambios guardados en " + state.csvPath});
        statusLabel->changeTextColor("BRIGHT_GREEN");
      }
    } else if (Input::lastChar == '4') {
      SceneManager::getInstance().stopRunning();
    }
  });

  auto hint = std::make_shared<NodePCT>(
      "Hint", Vec2{2, 57}, "WHITE", "BLACK",
      std::vector<std::string>{
          "Usa click del raton o los botones numerados [1-4] para navegar."});
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

  // ── Back button ───────────────────────────────────────────────────────
  root->setProcessFunction([&state](double) {
    if (Input::lastChar == 'm' || Input::lastChar == 'M') {
      SceneManager::getInstance().changeScene(createMainMenu(state));
    }
  });

  auto btnBack = std::make_shared<NodeButton>(
      "BtnBack", Vec2{5, 56}, "BLACK", "BRIGHT_WHITE",
      std::vector<std::string>{"  [M] Volver al menu  "});
  btnBack->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createMainMenu(state));
  });
  root->addChild(btnBack);

  return root;
}

// Scene 3 — Modify Member
inline std::shared_ptr<Node> createModifyScene(AppState &state) {
  auto root = std::make_shared<Node>("ModifyScene");

  root->addChild(makeBanner("Modificar Miembro de la Familia"));

  // Prompt for ID
  auto prompt = std::make_shared<NodePCT>(
      "Prompt", Vec2{2, 6}, "BRIGHT_WHITE", "BLACK",
      std::vector<std::string>{"  Ingresa el ID del miembro a modificar:"});
  root->addChild(prompt);

  // We use the process callback to perform blocking input once the scene has
  // had a chance to render at least once.
  auto logicNode = std::make_shared<Node>("ModifyLogic");
  auto frameWait = std::make_shared<int>(0);

  // Debug label to show what's happening
  auto debugLabel = std::make_shared<NodePCT>(
      "DebugLabel", Vec2{2, 30}, "BRIGHT_YELLOW", "BLACK",
      std::vector<std::string>{"Status: Esperando para iniciar..."});
  root->addChild(debugLabel);

  logicNode->setProcessFunction([&state, frameWait, debugLabel, root, logicNode](double) {
    if ((*frameWait) < 2) {
      (*frameWait)++;
      return;
    }

    // Now we can do blocking input
    // --- Step 1: read ID ---
    debugLabel->set_text({"Status: Ingresa ID en el prompt (arriba)..."});
    std::string idStr = Input::getLineInput({44, 6});
    int memberId = -1;
    try {
        if (!idStr.empty()) memberId = std::stoi(idStr);
    } catch (...) {
        memberId = -1;
    }

    debugLabel->set_text({"Status: Buscando ID " + std::to_string(memberId) + "..."});
    FamilyMember *member = state.tree.findMemberById(memberId);
    if (!member) {
      debugLabel->set_text({"Status: ERROR - ID " + std::to_string(memberId) + " no encontrado!"});
      debugLabel->changeTextColor("BRIGHT_RED");
      // Pause so user can see the error
      Input::getLineInput({2, 32}); // Wait for enter
      SceneManager::getInstance().changeScene(createMainMenu(state));
      return;
    }

    debugLabel->set_text({"Status: Modificando a " + member->name + " " + member->last_name});
    debugLabel->changeTextColor("BRIGHT_GREEN");

    // Update the field labels to show current data
    std::vector<std::string> currentVals = {
        member->name,
        member->last_name,
        std::string(1, member->gender),
        std::to_string(member->age),
        member->is_dead ? "1 (Si)" : "0 (No)",
        member->in_jail ? "1 (Si)" : "0 (No)"
    };

    for (int i = 0; i < 6; ++i) {
      auto fieldNode = std::dynamic_pointer_cast<NodePCT>(root->getChild(4 + i)); // Banner=0, Prompt=1, DebugLabel=2, LogicNode=3
      if (fieldNode) {
          std::string base = fieldNode->getText()[0];
          // Replace "Enter = mantener actual" with the actual value if possible, or just append
          fieldNode->set_text({base + "[" + currentVals[i] + "] "});
          fieldNode->changeTextColor("BRIGHT_WHITE");
      }
    }

    // Wait one more frame to render the updated labels
    (*frameWait) = 0;
    logicNode->setProcessFunction([&state, memberId, member, debugLabel](double) {
        // Nested lambda to continue after render
        // Step 2: input values
        auto getStr = [](Vec2 pos, const std::string &current) -> std::string {
          std::string val = Input::getLineInput(pos);
          return val.empty() ? current : val;
        };
        auto getIntOrKeep = [](Vec2 pos, int current) -> int {
          std::string raw = Input::getLineInput(pos);
          if (raw.empty()) return current;
          try { return std::stoi(raw); } catch (...) { return current; }
        };

        debugLabel->set_text({"Status: Ingresa nuevos datos o pulsa Enter para mantener los actuales."});

        std::string newName = getStr({55, 12}, member->name);
        std::string newLast = getStr({55, 14}, member->last_name);

        std::string gStr = Input::getLineInput({55, 16});
        char newGender = (gStr == "H" || gStr == "M") ? gStr[0] : member->gender;

        int newAge = getIntOrKeep({55, 18}, member->age);

        std::string deadStr = Input::getLineInput({55, 20});
        bool newDead = deadStr.empty() ? member->is_dead : (deadStr == "1");

        std::string jailStr = Input::getLineInput({55, 22});
        bool newJail = jailStr.empty() ? member->in_jail : (jailStr == "1");

        state.tree.updateMember(memberId, newName, newLast, newGender, newAge,
                                newDead, newJail);

        SceneManager::getInstance().changeScene(createMainMenu(state));
    });
  });
  root->addChild(logicNode);

  // Fields display (static labels — the blocking input will overlay them)
  std::vector<std::string> fields = {
      "  Nombre       (Enter = mantener actual): ",
      "  Apellido     (Enter = mantener actual): ",
      "  Genero H/M   (Enter = mantener actual): ",
      "  Edad         (Enter = mantener actual): ",
      "  Esta muerto? 0/1 (Enter = mantener):   ",
      "  En carcel?   0/1 (Enter = mantener):   ",
  };
  int yPos = 12;
  for (size_t i = 0; i < fields.size(); ++i) {
    root->addChild(std::make_shared<NodePCT>(
        "Field" + std::to_string(i), Vec2{2, yPos}, "BRIGHT_CYAN", "BLACK",
        std::vector<std::string>{fields[i]}));
    yPos += 2;
  }

  auto btnBack = std::make_shared<NodeButton>(
      "BtnBack", Vec2{5, 56}, "BLACK", "BRIGHT_WHITE",
      std::vector<std::string>{"  [M] Volver al menu  "});
  btnBack->setOnClick([&state]() {
    SceneManager::getInstance().changeScene(createMainMenu(state));
  });
  // Handle 'M' key
  root->setProcessFunction([&state](double) {
    if (Input::lastChar == 'm' || Input::lastChar == 'M') {
      SceneManager::getInstance().changeScene(createMainMenu(state));
    }
  });
  root->addChild(btnBack);

  return root;
}

#endif // SCENES_HPP
