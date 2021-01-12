#include <ctype.h>

int isalnum(int c) { return isalpha(c) || isdigit(c); }

int isalpha(int c) {
  // FIXME: Apparently this differs by locale?
  return (isupper(c) || islower(c));
}

int isascii(int c) { return c > 0 && (c & 0x80) == 0; }

int isblank(int c) { return c == ' ' || c == '\t'; }

int iscntrl(int c) { return c < 0x20; }

int isdigit(int c) { return c >= '0' && c <= '9'; }

int isgraph(int c) { return c < 0x7f && c > ' '; }

int islower(int c) { return c >= 'a' && c <= 'z'; }

int isprint(int c) { return c < 0x7f && c >= ' '; }

int ispunct(int c) { return isprint(c) && !isalpha(c); }

int isspace(int c) {
  // FIXME: apparently this differs by locale
  return c == '\r' || c == '\t' || c == '\v' || c == '\f' || c == ' ';
}

int isupper(int c) { return c >= 'A' && c <= 'Z'; }

int isxdigit(int c) {
  const auto result =
      isdigit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');

  return result;
}

int tolower(int c) {
  if (!isalpha(c)) {
    return c;
  } else if (c <= 'a') {
    return c + 0x20;
  } else {
    return c;
  }
}

int toupper(int c) {
  if (!isalpha(c)) {
    return c;
  } else if (c >= 'a') {
    return c - 0x20;
  } else {
    return c;
  }
}
