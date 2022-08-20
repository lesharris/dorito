#pragma once

#include "Widget.h"

#include <cstdint>
#include <string>
#include <vector>

namespace dorito {

  class MonitorsWidget : public Widget {
  public:
    MonitorsWidget();

    std::string Name() override {
      return "MonitorsWidget";
    }

    void Draw() override;

  public:
    enum class Type {
      Register = 0,
      Memory
    };

    struct MonitorItem {
      Type type;
      int32_t base;
      int32_t length;
      std::string format;
      std::string name;
    };

  private:
    struct FormatItem {
      std::string format;
      std::string value;
    };

  private:
    std::string ParseFormat(const MonitorItem &item);

  private:
    void HandleClearMonitors(const Events::UIClearMonitors &event);

    void HandleAddMonitor(const Events::UIAddMonitor &event);

  private:
    std::vector<MonitorItem> m_Monitors;
  };

} // dorito
