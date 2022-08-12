#pragma once

#include <string>
#include <utility>

namespace dorito {

  class GameLayer {
  public:
    explicit GameLayer(std::string name = "State") : m_Name(std::move(name)) {}

    virtual ~GameLayer() = default;

    [[maybe_unused]] virtual void OnAttach() {}

    [[maybe_unused]] virtual void OnDetach() {}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif

    virtual void Update(double timestep) {}

#ifdef _MSC_VER
#pragma warning(default:4100)
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    virtual void Render() {}

    [[maybe_unused]] virtual void RenderTexture() {}

    [[maybe_unused]] virtual void BeginFrame() {}

    [[maybe_unused]] virtual void EndFrame() {}

    [[maybe_unused]] virtual bool RenderTarget() { return false; }

    [[maybe_unused]] [[nodiscard]] virtual std::string Name() const noexcept {
      return m_Name;
    }

  protected:
    std::string m_Name;
  };

} // dorito

