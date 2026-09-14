#include <libheif/heif.h>

#include <iostream>

int main() {
  const char* version = heif_get_version();
  if (version == nullptr || version[0] == '\0') {
    return 1;
  }

  std::cout << "libheif " << version << '\n';
  return 0;
}
