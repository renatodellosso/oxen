int i = 0;
int low = 0;
int middle = 0;
int high = 0;

while (i < 120) {
  if (i < 4) {
    low = low + 1;
  } else if (i < 8) {
    middle = middle + 1;
  } else {
    high = high + 1;
  }
  i = i + 1;
}

print low + middle + high;
