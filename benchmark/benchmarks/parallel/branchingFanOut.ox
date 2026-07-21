int a = 0;
int b = 0;
int c = 0;
int d = 0;
int e = 0;
int f = 0;
int g = 0;
int h = 0;

int ai = 0;
int bi = 0;
int ci = 0;
int di = 0;
int ei = 0;
int fi = 0;
int gi = 0;
int hi = 0;

while (ai < 4000) {
  if (ai < 2000) a = a + 2;
  else a = a - 1;
  ai = ai + 1;
}

while (bi < 4000) {
  if (bi < 2100) b = b + 2;
  else b = b - 1;
  bi = bi + 1;
}

while (ci < 4000) {
  if (ci < 2200) c = c + 2;
  else c = c - 1;
  ci = ci + 1;
}

while (di < 4000) {
  if (di < 2300) d = d + 2;
  else d = d - 1;
  di = di + 1;
}

while (ei < 4000) {
  if (ei < 2400) e = e + 2;
  else e = e - 1;
  ei = ei + 1;
}

while (fi < 4000) {
  if (fi < 2500) f = f + 2;
  else f = f - 1;
  fi = fi + 1;
}

while (gi < 4000) {
  if (gi < 2600) g = g + 2;
  else g = g - 1;
  gi = gi + 1;
}

while (hi < 4000) {
  if (hi < 2700) h = h + 2;
  else h = h - 1;
  hi = hi + 1;
}

print a + b + c + d + e + f + g + h;
