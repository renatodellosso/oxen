string lane01 = "q013456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane02 = "q023456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane03 = "q033456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane04 = "q043456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane05 = "q053456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane06 = "q063456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane07 = "q073456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string lane08 = "q083456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

int index01 = 0;
int index02 = 0;
int index03 = 0;
int index04 = 0;
int index05 = 0;
int index06 = 0;
int index07 = 0;
int index08 = 0;

while (index01 < 9) {
  lane01 = lane01 + lane01 + lane01 + lane01;
  index01 = index01 + 1;
}

while (index02 < 9) {
  lane02 = lane02 + lane02 + lane02 + lane02;
  index02 = index02 + 1;
}

while (index03 < 9) {
  lane03 = lane03 + lane03 + lane03 + lane03;
  index03 = index03 + 1;
}

while (index04 < 9) {
  lane04 = lane04 + lane04 + lane04 + lane04;
  index04 = index04 + 1;
}

while (index05 < 9) {
  lane05 = lane05 + lane05 + lane05 + lane05;
  index05 = index05 + 1;
}

while (index06 < 9) {
  lane06 = lane06 + lane06 + lane06 + lane06;
  index06 = index06 + 1;
}

while (index07 < 9) {
  lane07 = lane07 + lane07 + lane07 + lane07;
  index07 = index07 + 1;
}

while (index08 < 9) {
  lane08 = lane08 + lane08 + lane08 + lane08;
  index08 = index08 + 1;
}

print index01 + index02 + index03 + index04 + index05 + index06 + index07 + index08;

