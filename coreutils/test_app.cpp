#include <fnctl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  size_t index = 0;
  char buf[128];

  const int fd = open("/dev/keyboard", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  while (1) {
    const int n = read(fd, buf + index, 1);
    if (n < 1) {
      perror("read");
      break;
    }
    if (buf[index] == '\n') {
      buf[index] = '\0';
      printf("Hello %s, my name is world!\n", buf);
      break;
    } else {
      index++;
    }
  }

  if (close(fd)) {
    perror("close");
  }
}
