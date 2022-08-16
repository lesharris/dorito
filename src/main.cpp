#include "core/Dorito.h"

int main() {
  auto &app = dorito::Dorito::Get();

  app.Run();

  return 0;
}
