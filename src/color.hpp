#pragma once

#include <string>

#define COLOR_ENABLED

const int BG_OFFSET = 40;
const int FG_OFFSET = 30;

enum class Color {
  Black = 0,
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Magenta = 5,
  Cyan = 6,
  White = 7,
};

enum class SpecialCodes {
  Reset = 0,
  Bold = 1,
  Underline = 4,
  Inverse = 7,
  BoldOff = 21,
  UnderlineOff = 24,
  InverseOff = 27,
};

std::string colorize(std::string text, Color fg, Color bg = Color::Black,
                     SpecialCodes special = SpecialCodes::Reset);