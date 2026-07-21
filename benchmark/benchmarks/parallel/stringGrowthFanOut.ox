string payload = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
payload = payload + payload;
payload = payload + payload;
payload = payload + payload;
payload = payload + payload;
payload = payload + payload;
payload = payload + payload;

string a = "";
string b = "";
string c = "";
string d = "";
string e = "";
string f = "";
string g = "";
string h = "";

int ai = 0;
int bi = 0;
int ci = 0;
int di = 0;
int ei = 0;
int fi = 0;
int gi = 0;
int hi = 0;

while (ai < 200) {
  a = a + payload;
  ai = ai + 1;
}

while (bi < 200) {
  b = b + payload;
  bi = bi + 1;
}

while (ci < 200) {
  c = c + payload;
  ci = ci + 1;
}

while (di < 200) {
  d = d + payload;
  di = di + 1;
}

while (ei < 200) {
  e = e + payload;
  ei = ei + 1;
}

while (fi < 200) {
  f = f + payload;
  fi = fi + 1;
}

while (gi < 200) {
  g = g + payload;
  gi = gi + 1;
}

while (hi < 200) {
  h = h + payload;
  hi = hi + 1;
}

print a == h;
print b == g;
print c == f;
print d == e;
