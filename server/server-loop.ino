WiFiServer server(80);

bool server_init(void) {
  if (!SPIFFS.begin(true)) {
    return false;
  }

  if (!wifi_connect()) {
    return false;
  }

  display_str("Connecting...");

  MDNS.begin("tree");

  IPAddress ip = WiFi.localIP();
  server.begin();

  MDNS.addService("http", "tcp", 80);

  display_str("Connected at " + ip.toString());
  
  int status = xTaskCreate(server_loop, "ServerLoop", SERVER_LOOP_STACK_SIZE, (void *) NULL, tskIDLE_PRIORITY, NULL);

  if (status != pdPASS) {
    return false;
  }

  return true;
}

bool wifi_connect(void) {
  char ssid[STRING_LEN];
  File file = SPIFFS.open("/ssid.txt", FILE_READ);
  read_file(&file, (uint8_t *) ssid, file.size());
  ssid[file.size()] = '\0';
  
  char password[STRING_LEN];
  file = SPIFFS.open("/pass.txt", FILE_READ);
  read_file(&file, (uint8_t *) password, file.size());
  password[file.size()] = '\0';

  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    if (counter >= 2 * WIFI_CONNECT_TIMEOUT_SECONDS) {
      return false;
    }
  }

  return true;
}

void server_loop(void *params) {
  while (true) {
    WiFiClient client = server.available();
      
    if (!client.connected()) {
      continue;
    }

    Request request;

    if (parse_request(client, &request)) {
      serve_request(client, &request);
      free_request(&request);    
    } else {
      Response response;

      create_response(&response);
      strcpy(response.status, "500 Internal Server Error");

      send_response(client, &response);
    }
  }
}

void serve_request(WiFiClient client, Request *request) {
  Response response;
  
  create_response(&response);

  int status = HTTP_NOT_FOUND;

  if (strcmp(request->method, "GET") == 0) {
    status = serve_get(request, &response);
  } else if (strcmp(request->method, "POST") == 0) {
    status = serve_post(request, &response);
  }

  set_status(&response, status);
  send_response(client, &response);
}

void send_response(WiFiClient client, Response *response) {
  client.write(response->version, strlen(response->version));
  client.print(" ");

  client.write(response->status, strlen(response->status));
  client.println("");

  for (int j = 0; j < response->n_headers; j ++) {
    Header *header = &response->headers[j];

    client.write(header->key, strlen(header->key));
    client.print(": ");

    client.write(header->value, strlen(header->value));

    client.println("");
  }

  client.println("");

  if (response->body) {
    client.write(response->body, response->body_len);
  }

  client.clear();
  client.stop();

  free_response(response);
}