#include "defs.h"

void GridTr_ssplit(const char *str, const char *delims,
                   struct ssplit_s *split) {
  if (!str || !split) {
    return;
  }

  // Build delimiter table (reentrant: all local)
  unsigned char is_delim[256] = {0};
  for (const uint8 *d = (const uint8 *)delims; *d; ++d) {
    is_delim[*d] = 1;
  }

  // Copy str -> buf, replacing delimiters with '\0'
  int n = 0;
  for (; str[n] != '\0' && n < (int)sizeof(split->buf) - 1; ++n) {
    uint8 c = (uint8)str[n];
    split->buf[n] = is_delim[c] ? '\0' : (char)c;
  }
  split->buf[n] = '\0';

  // Collect token starts
  int i = 0;

  // Skip leading '\0's
  while (i < n && split->buf[i] == '\0')
    ++i;

  while (i < n && split->num_toks < 32) {
    split->toks[split->num_toks++] = &split->buf[i];

    // Advance to end of token
    while (i < n && split->buf[i] != '\0')
      ++i;

    // Skip delimiters (now '\0's) to next token
    while (i < n && split->buf[i] == '\0')
      ++i;
  }
}