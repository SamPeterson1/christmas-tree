/* Respond to GET requests */
int serve_get(Request *request, Response *response) {
  if (strcmp(request->path, "/") == 0) {
    return serve_file(response, "/index.html", "text/html");
  } else if (strcmp(request->path, "/effect.js") == 0) {
    return serve_file(response, "/effect.js", "application/javascript");
  } else if (strcmp(request->path, "/style.css") == 0) {
    return serve_file(response, "/style.css", "text/css");
  } else if (strcmp(request->path, "/favicon.ico") == 0) {
    return serve_file(response, "/favicon.ico", "image/x-icon");
  } else if (strcmp(request->path, "/background.jpg") == 0) {
    return serve_file(response, "/background.jpg", "image/jpg");
  }

  return HTTP_NOT_FOUND;
}

/* Serve the content of a file */
int serve_file(Response *response, char *path, char *content_type) {
  uint32_t content_size = 0;
  uint8_t *content = read_content(path, &content_size);

  add_content_headers(response, content_size, content_type);

  response->body = (char *) content;
  response->body_len = content_size;

  return HTTP_OK;
}