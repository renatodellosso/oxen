int value = 0;
int i = 0;

while (i < 250) {
  if (i < 125) {
    value = value + 2;
  } else {
    value = value + 3;
  }
  i = i + 1;
}

print value;
