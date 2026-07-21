int left = 0;
int right = 0;

void updateLeft(int value) {
  left = left + value;
}

void updateRight(int value) {
  right = right + value * 2;
}

int i = 0;
while (i < 100) {
  updateLeft(i);
  updateRight(i);
  i = i + 1;
}

print left + right;
