string a = "a123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string b = "b123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string c = "c123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string d = "d123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string e = "e123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string f = "f123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string g = "g123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
string h = "h123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

int ai = 0;
int bi = 0;
int ci = 0;
int di = 0;
int ei = 0;
int fi = 0;
int gi = 0;
int hi = 0;

while (ai < 18) {
  a = a + a;
  ai = ai + 1;
}

while (bi < 18) {
  b = b + b;
  bi = bi + 1;
}

while (ci < 18) {
  c = c + c;
  ci = ci + 1;
}

while (di < 18) {
  d = d + d;
  di = di + 1;
}

while (ei < 18) {
  e = e + e;
  ei = ei + 1;
}

while (fi < 18) {
  f = f + f;
  fi = fi + 1;
}

while (gi < 18) {
  g = g + g;
  gi = gi + 1;
}

while (hi < 18) {
  h = h + h;
  hi = hi + 1;
}

print ai + bi + ci + di + ei + fi + gi + hi;
