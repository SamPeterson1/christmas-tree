/* 
 * Copy a substring of src into dst
 * start is inclusive, end is exclusive
 */
void substr(char *src, char *dst, int start, int end) {
  strncpy(dst, src + start, end - start);
  dst[end - start] = '\0';
}

/* Check if needle exists in haystack at index i */
bool find(char *haystack, char *needle, int i) {
  int needle_len = strlen(needle);
  int haystack_len = strlen(haystack);

  for (int j = 0; j < needle_len; j ++) {
    if (i + j >= haystack_len || haystack[i + j] != needle[j]) {
      return false;
    }
  }

  return true;
}

/* Split src into dst by matching substrings to split */
bool string_split(char *src, char **dst, char *split, int max_n_splits, int *n_splits) {
  *n_splits = 0;

  int src_len = strlen(src);
  int split_len = strlen(split);
  
  int left = 0;

  for (int i = 0; i < src_len; i ++) {
    if (*n_splits >= max_n_splits) {
      return false;
    }

    if (find(src, split, i)) {
      if (i - left >= STRING_LEN) {
        return false;
      }

      substr(src, dst[(*n_splits) ++], left, i);

      i += split_len - 1;
      left = i + 1;
    }
  }

  if (*n_splits >= max_n_splits || src_len - left >= STRING_LEN) {
    return false;
  }

  substr(src, dst[(*n_splits) ++], left, src_len);

  return true;
}

/* Convert alphabetic characters in a string to lowercase */
void to_lowercase(char *str) {
  for (int i = 0; i < strlen(str); i ++) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      str[i] += 'a' - 'A';
    }
  }
}
