#include "color.hpp"

static std::string getEscapeSequence(int code) {
  return "\033[" + std::to_string(code) + "m";
}

std::string colorize(std::string text, Color fg, Color bg,
                     SpecialCodes special) {
  return getEscapeSequence(static_cast<int>(special)) +
         getEscapeSequence(static_cast<int>(fg) + FG_OFFSET) +
         getEscapeSequence(static_cast<int>(bg) + BG_OFFSET) + text +
         getEscapeSequence(static_cast<int>(SpecialCodes::Reset));
}