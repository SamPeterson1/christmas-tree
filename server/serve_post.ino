/* Respond to POST requests */
int serve_post(Request *request, Response *response) {
  if (strcmp(request->path, "/flash") == 0) {
    return serve_flash(request, response);
  } else if (strcmp(request->path, "/uart") == 0) {
    return serve_uart(request, response);
  } else if (strcmp(request->path, "/reset") == 0) {
    reset();
    return HTTP_OK;
  } else if (strcmp(request->path, "/wifi/ssid") == 0) {
    return update_file(request, "text/plain", "/ssid.txt");
  } else if (strcmp(request->path, "/wifi/password") == 0) {
    return update_file(request, "text/plain", "/pass.txt");
  } else if (strcmp(request->path, "/save") == 0) {
    return serve_save(request, response);
  } else if (strcmp(request->path, "/load") == 0) {
    return serve_load(request, response);
  } else if (strcmp(request->path, "/update/") == 0) {
    return update_file(request, "text/html", "/index.html");
  } else if (strcmp(request->path, "/update/effect.js") == 0) {
    return update_file(request, "application/javascript", "/effect.js");
  } else if (strcmp(request->path, "/update/style.css") == 0) {
    return update_file(request, "text/css", "/style.css");
  } else if (strcmp(request->path, "/update/favicon.ico") == 0) {
    return update_file(request, "image/x-icon", "/favicon.ico");
  } else if (strcmp(request->path, "/update/background.jpg") == 0) {
    return update_file(request, "image/jpg", "/background.jpg");
  }

  return HTTP_NOT_FOUND;
}

/* 
 * Write content in the Request to a file
 * Check if the content type is appropriate
 */
int update_file(Request *request, char *content_type, char *file_name) {
  if (strcmp(request->content.type, content_type) != 0) {
    return HTTP_BAD_REQUEST;
  }

  write_content(file_name, &request->content);

  return HTTP_OK;
}

/*
 * File format of saves.dat
 *
 * Per effect, the saves are stored using the following bytes:
 * Byte 0: Effect id
 * Byte 1: Effect version
 * Byte 2: Parameter length in bytes
 * Bytes 3..n: Parameters
 *
 * Each effect saved (in the above format) is concatenated to form the entire save file
 * Effect ids should be unique within the file, but do not need to be in order
 */

/*
 * Given a byte array in the format of saves.dat
 * find an effect with matching id and version
 */
bool find_effect(uint8_t *buf, uint8_t buf_size, uint8_t effect_id, uint8_t effect_version, uint32_t *effect_index, uint32_t *effect_size, bool version_sensitive) {
  int i = 0;

  while (i < buf_size) {
    uint8_t id = buf[i ++];
    uint8_t version = buf[i ++];

    if (id == effect_id && (!version_sensitive || version == effect_version)) {
      *effect_index = i - 2;
    }

    uint16_t params_len = buf[i ++];

    i += params_len;

    if (id == effect_id && (!version_sensitive || version == effect_version)) {
      *effect_size = params_len + 3;
      return true;
    }
  }

  return false;
}

/*
 * Load saved effect parameters if they exist
 * First byte of request is the effect id
 * Second byte of request is the effect version
 */
int serve_load(Request *request, Response *response) {
  if (strcmp(request->content.type, "application/octet-stream") != 0) {
    return HTTP_BAD_REQUEST;
  }

  int content_size = request->content.len;

  if (content_size != 2) {
    return HTTP_BAD_REQUEST;
  }

  uint8_t effect_id = request->content.bytes[0];
  uint8_t effect_version = request->content.bytes[1];

  File file = SPIFFS.open("/saves.dat", FILE_READ);

  uint32_t size = file.size();
  uint8_t *buf = new uint8_t[size];

  read_file(&file, buf, file.size());

  file.close();

  uint32_t effect_index = 0;
  uint32_t effect_size = 0;

  if (!find_effect(buf, size, effect_id, effect_version, &effect_index, &effect_size, true)) {
    return HTTP_NOT_FOUND;
  }

  char *bytes = (char *) malloc(effect_size - 1);

  /* Copy effect data to response body buffer*/
  int i = 0;
  for (int j = 0; j < effect_size; j ++) {
    /* Skip parameter length - this is not required to be sent to the led controller */
    if (j == 2) {
      continue;
    }

    bytes[i ++] = buf[j + effect_index];
  }

  response->body = bytes;
  response->body_len = i;

  add_content_headers(response, effect_size - 1, "application/octet-stream");
  
  return HTTP_OK;
}

/*
 * Create new save or overwrite existing save with data in Request
 * First byte of request is the effect id
 * Second byte of request is the effect version
 * Remainig bytes are the effect parameters
 */
int serve_save(Request *request, Response *response) {
  if (strcmp(request->content.type, "application/octet-stream") != 0) {
    return HTTP_BAD_REQUEST;
  }

  uint8_t effect_id = request->content.bytes[0];
  uint8_t effect_version = request->content.bytes[1];

  File file = SPIFFS.open("/saves.dat", FILE_READ, true);

  uint32_t size = file.size();
  uint8_t *buf = (uint8_t *) malloc(size);

  read_file(&file, buf, size);

  file.close();

  uint32_t effect_index = 0;
  uint32_t effect_size = 0;

  find_effect(buf, size, effect_id, effect_version, &effect_index, &effect_size, false);

  uint32_t new_size = size - effect_size + request->content.len + 1;
  uint8_t *new_buf = (uint8_t *) malloc(new_size);

  int i = 0;

  for (int j = 0; j < size; j ++) {
    if (j - effect_index >= 0 && j - effect_index < effect_size) {
      continue;
    }

    new_buf[i ++] = buf[j];
  }

  for (int j = 0; j < request->content.len; j ++) {
    new_buf[i ++] = request->content.bytes[j];

    if (j == 1) {
      new_buf[i ++] = request->content.len - 2;
    }
  }

  file = SPIFFS.open("/saves.dat", FILE_WRITE);

  write_file(&file, new_buf, new_size);
  file.close();

  free(buf);
  free(new_buf);

  return HTTP_OK;
}

/*
 * Send and recieve UART data to/from the led controller
 * Request content is simply the bytes to be sent
 * The server will first send the bytes in the request over UART
 * The led controller will then reply with 2 bytes (little endian) indicating the length of data to expect
 * and then the data itself
 */
int serve_uart(Request *request, Response *response) {
  if (strcmp(request->content.type, "application/octet-stream") != 0) {
    return HTTP_BAD_REQUEST;
  }

  uint32_t n_bytes = request->content.len;

  Serial.write(request->content.bytes, n_bytes);

  uint8_t bytes[UART_RX_BUF_SIZE];

  uint8_t size_bytes[2];

  if (Serial.readBytes(size_bytes, 2) != 2) {
    return HTTP_INTERNAL_SERVER_ERROR;
  }
  
  n_bytes = (size_bytes[1] << 8) | size_bytes[0];
  
  if (Serial.readBytes(bytes, n_bytes) != n_bytes) {
    return HTTP_INTERNAL_SERVER_ERROR;
  }

  char *body = (char *) malloc(n_bytes);
  memcpy(body, bytes, n_bytes);

  response->body = body;
  response->body_len = n_bytes;

  add_content_headers(response, n_bytes, "application/octet-stream");

  return HTTP_OK;
}

/*
 * Use the bootloader on the led controller to flash a program
 * Request is multipart/form-data encoded
 */
int serve_flash(Request *request, Response *response) {
  int n_newlines = 0;
  int i = 0;
  int n_splits = 0;

  int content_length_index = find_header(request, "content-length");
  int content_type_index = find_header(request, "content-type");

  char _dummy[STRING_LEN];
  char boundary[STRING_LEN];

  char *content_type_split[2] = {_dummy, boundary};
  
  if (!string_split(request->headers[content_type_index].value, content_type_split, "; ", 2, &n_splits) || n_splits != 2) {
    return HTTP_INTERNAL_SERVER_ERROR;
  }

  /* Hacky solution to skip exactly to the data in the body without properly parsing it */
  int content_length = atoi(request->headers[content_length_index].value);
  int max_i = content_length - (strlen(boundary) - 9) - 8;
  
  while (n_newlines < 4) {
    if (request->content.bytes[i ++] == '\n') {
      n_newlines ++;
    }
  }

  if (flash_program((uint8_t *) (request->content.bytes + i), max_i - i)) {
    return HTTP_OK;
  } else {
    return HTTP_INTERNAL_SERVER_ERROR;
  }
}