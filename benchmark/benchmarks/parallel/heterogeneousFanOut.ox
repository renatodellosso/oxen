int additive = 0;
int subtractive = 80;
int multiplied = 1;
int divided = 61;
int lowBranch = 0;
int highBranch = 0;
int narrowStep = 0;
int wideStep = 0;

int ai = 0;
int si = 0;
int mi = 0;
int di = 0;
int li = 0;
int hi = 0;

while (ai < 60) {
  additive = additive + 1;
  ai = ai + 1;
}

while (si < 60) {
  subtractive = subtractive - 1;
  si = si + 1;
}

while (mi < 60) {
  multiplied = multiplied * 2;
  multiplied = multiplied / 2;
  mi = mi + 1;
}

while (di < 60) {
  divided = divided / 1;
  divided = divided - 1;
  di = di + 1;
}

while (li < 60) {
  if (li < 30) lowBranch = lowBranch + 2;
  else lowBranch = lowBranch - 1;
  li = li + 1;
}

while (hi < 60) {
  if (hi > 30) highBranch = highBranch + 2;
  else highBranch = highBranch + 1;
  hi = hi + 1;
}

while (narrowStep < 60) {
  narrowStep = narrowStep + 1;
}

while (wideStep < 60) {
  wideStep = wideStep + 3;
}

print additive + subtractive + multiplied + divided + lowBranch + highBranch + narrowStep + wideStep;
