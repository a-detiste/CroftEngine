#pragma once

#include "selectedmenustate.h"

#include <gslu.h>
#include <memory>
#include <vector>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace ui::widgets
{
class TabBox;
}

namespace engine
{
class Engine;
}

namespace menu
{
struct MenuDisplay;
struct MenuRingTransform;
class MenuState;

class RenderSettingsMenuState : public SelectedMenuState
{
private:
  class CheckListBox;
  std::vector<std::shared_ptr<CheckListBox>> m_listBoxes{};
  std::unique_ptr<MenuState> m_previous;
  gslu::nn_unique<ui::widgets::TabBox> m_tabs;

public:
  explicit RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::Engine& engine);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  ~RenderSettingsMenuState() override;
};
} // namespace menu
