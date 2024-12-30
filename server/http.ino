/*
 * Only headers that match a string in this list will be stored in a Request
 * Done to save memory
 */
const char* accepted_headers[N_ACCEPTED_HEADERS] = {
  "content-length",
  "content-type"
};

/*
 * Initialize a bare mimimum Response struct
 * Default response status is 500
 */
void create_response(Response *response) {
  response->body = nullptr;
  response->body_len = 0;
  response->n_headers = 0;
  strcpy(response->status, "500 Internal Server Error");
  strcpy(response->version, "HTTP/1.1");
}

/* Free malloc'd memory in a Request */
void free_request(Request *request) {
  free(request->content.bytes);
}

/* Free malloc'd memory in a Response */
void free_response(Response *response) {
  free(response->body);
}

/* Given a status code, set the status string of a Response */
void set_status(Response *response, int status_code) {
  switch (status_code) {
    case HTTP_OK:
      strcpy(response->status, "200 OK");
      break;
    case HTTP_BAD_REQUEST:
      strcpy(response->status, "400 Bad Request");
      break;
    case HTTP_NOT_FOUND:
      strcpy(response->status, "404 Not Found");
      break;
    case HTTP_INTERNAL_SERVER_ERROR:
      strcpy(response->status, "500 Internal Server Error");
      break;
    default:
      strcpy(response->status, "500 Internal Server Error");
  }
}

/* Set content-length and content-type headers */
bool add_content_headers(Response *response, int content_length, char *content_type) {
  if (!add_header(response, "Content-Type", content_type)) {
    return false;
  }

  char content_length_str[STRING_LEN];
  itoa(content_length, content_length_str, 10);

  if (!add_header(response, "Content-Length", content_length_str)) {
    return false;
  }

  return true;
}

/* 
 * Add a header to a Response 
 * Returns false if exceeds MAX_N_HEADERS
*/
bool add_header(Response *response, char *key, char *value) {
  if (response->n_headers >= MAX_N_HEADERS) {
    return false;
  }

  strcpy(response->headers[response->n_headers].key, key);
  strcpy(response->headers[response->n_headers].value, value);

  response->n_headers ++;

  return true;
}

/* 
 * Add a header to a Request 
 * Returns false if exceeds MAX_N_HEADERS
*/
bool add_header(Request *request, char *key, char *value) {
  if (request->n_headers >= MAX_N_HEADERS) {
    return false;
  }

  strcpy(request->headers[request->n_headers].key, key);
  strcpy(request->headers[request->n_headers].value, value);

  request->n_headers ++;

  return true;
}

/*
 * Get the index of a header with a certain key
 * Returns -1 if not found
 */
int find_header(Request *request, char *header) {
  for (int i = 0; i < request->n_headers; i ++) {
    if (strcmp(request->headers[i].key, header) == 0) {
      return i;
    }
  }

  return -1;
}

/* 
 * Wait for client data to become available
 * Timeout is WIFI_CLIENT_TIMEOUT_SECONDS
 */
bool wait_on_client(WiFiClient client) {
  int counter = 0;
  while (client.available() == 0 && counter < 100 * WIFI_CLIENT_TIMEOUT_SECONDS) {
    counter ++;
    delay(10);
  }

  return counter < 100 * WIFI_CLIENT_TIMEOUT_SECONDS;
}

/* 
 * Read characters until '\n' is encountered
 * Ignores '\r'
 * Returns false if client times out or line exceeds max_size
 */
bool read_line(WiFiClient client, char *line, uint32_t max_size) {
  int i = 0;

  while (true) {
    if (i == max_size) {
      return false;
    }

    if (!wait_on_client(client)) {
      return false;
    }

    char c = client.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      line[i ++] = '\0';
      break;
    }

    line[i ++] = c;
  }

  return true;
}

/* 
 * Parse first line of an http request 
 * Example: "POST /uart HTTP/1.1"
 */
bool parse_method(WiFiClient client, Request *request) {
  char line[STRING_LEN];

  if (!read_line(client, line, STRING_LEN)) {
    return false;
  }
  
  int n_splits = 0;

  char method[STRING_LEN];
  char path[STRING_LEN];
  char version[STRING_LEN];

  char *tokens[3] = {method, path, version};

  if (!string_split(line, tokens, " ", 3, &n_splits) || n_splits != 3) {
    return false;
  }

  strcpy(request->method, method);
  strcpy(request->path, path);
  strcpy(request->version, version);

  return true;
}

/* Check if a header is in the accepted_headers list */
bool header_accepted(char *key) {
  for (int j = 0; j < N_ACCEPTED_HEADERS; j ++) {
    if (strcmp(accepted_headers[j], key) == 0) {
      return true;
    }
  }

  return false;
}

/* 
 * Parse next header into a Request 
 * Example: "Content-Type: application/json"
 */
bool parse_header(Request *request, char *line) {
  int n_splits;

  char key[STRING_LEN];
  char value[STRING_LEN];

  char *key_value[2] = {key, value};

  if (!string_split(line, key_value, ": ", 2, &n_splits) || n_splits != 2) {
    return false;
  }

  to_lowercase(key);

  if (header_accepted(key) && !add_header(request, key, value)) {
    return false;
  }

  return true;
}

/*
 * Parse all headers into a Request
 * Stops after two consecutive newline characters
 */
bool parse_headers(WiFiClient client, Request *request) {
  char line[2 * STRING_LEN];

  while (true) {
    if (!read_line(client, line, 2 * STRING_LEN)) {
      return false;
    }

    if (strlen(line) == 0) {
      break;
    }

    if (!parse_header(request, line)) {
      return false;
    }
  }

  return true;
}

/*
 * Parse string representation of content_length into an integer
 * Return false if there would be an overflow or the content_length header is otherwise invalid
 */
bool parse_content_length(char *str_length, uint32_t *length) {
  /* Prevent overflow of atoi */
  if (strlen(str_length) >= MAX_CONTENT_LEN_DIGITS) {
    return false;
  }

  /* Verify content-length is numeric */
  for (int i = 0; i < strlen(str_length); i ++) {
    if (str_length[i] < '0' || str_length[i] > '9') {
      return false;
    }
  }

  *length = atoi(str_length);

  if (*length >= MAX_CONTENT_LEN) {
    return false;
  }

  return true;
}

/*
 * Read a number of bytes into a buffer
 * Return false if client times out
 */
bool read_bytes(WiFiClient client, uint8_t *bytes, uint32_t len) {
  for (int i = 0; i < len; i ++) {
    if (!wait_on_client(client)) {
      return false;
    }

    bytes[i] = client.read();
  }

  return true;
}

/*
 * Parse content into a Request
 * Return false if client times out or invalid content-length header
 */
bool parse_content(WiFiClient client, Request *request) {
  int content_type_index = find_header(request, "content-type");
  int content_length_index = find_header(request, "content-length");

  /* No content exists */
  if (content_type_index == -1 || content_length_index == -1) {
    strcpy(request->content.type, "");
    request->content.len = 0;
    request->content.bytes = 0;

    return true;
  }

  char *content_length_value = request->headers[content_length_index].value;
  uint32_t len = 0;

  if (!parse_content_length(content_length_value, &len)) {
    return false;
  }

  uint8_t *bytes = (uint8_t *) malloc(len);

  if (!read_bytes(client, bytes, len)) {
    return false;
  }
  
  strcpy(request->content.type, request->headers[content_type_index].value);
  request->content.len = len;
  request->content.bytes = bytes;

  return true;
}

/* Parse an http request */
bool parse_request(WiFiClient client, Request *request) {
  if (!parse_method(client, request)) return false;
  if (!parse_headers(client, request)) return false;
  if (!parse_content(client, request)) return false;

  return true;
}