#include <WiFi.h>
#include <Wire.h>               
#include "HT_SSD1306Wire.h"
#include "SPIFFS.h"
#include "ESPmDNS.h"
#include "stdint.h"
#include "Arduino.h"

#define BOOT0_PIN 12
#define BOOT1_PIN 13
#define NRST_PIN 25

#define FLASH_BOOT_UART 0x7F

#define FLASH_CMD_GET 0x00
#define FLASH_CMD_EXT_ERASE_MEMORY 0x44
#define FLASH_CMD_WRITE_MEMORY 0x31
#define FLASH_CMD_READ_MEMORY 0x11

#define FLASH_ACK 0x79
#define FLASH_NACK 0x1F

#define SERVER_LOOP_STACK_SIZE 32768
#define UART_RX_BUF_SIZE 2048

#define MAX_N_HEADERS 4
#define STRING_LEN 128

#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_NOT_FOUND 404
#define HTTP_BAD_REQUEST 400

#define HTTP_OK 200

#define WIFI_CONNECT_TIMEOUT_SECONDS 10

#define WIFI_CLIENT_TIMEOUT_SECONDS 5
#define MAX_CONTENT_LEN_DIGITS 6
#define MAX_CONTENT_LEN 16384
#define N_ACCEPTED_HEADERS 2

struct Content {
  char type[STRING_LEN];
  uint32_t len;
  uint8_t *bytes;
};

struct Header {
  char key[STRING_LEN];
  char value[STRING_LEN];
};

struct Request {
  Content content;
  Header headers[MAX_N_HEADERS];
  int n_headers = 0;
  char method[STRING_LEN];
  char path[STRING_LEN];
  char version[STRING_LEN];
};

struct Response {
  Header headers[MAX_N_HEADERS];
  int n_headers = 0;
  char status[STRING_LEN];
  char version[STRING_LEN];
  uint32_t body_len = 0;
  char *body;
};

/* flash.ino */
void flash_init(void);
void enter_bootloader(void);
void exit_bootloader(void);
void reset(void);
bool flash_program(uint8_t *bytes, uint32_t n_bytes);
bool init_bootloader(void);
bool check_ack(void);
bool write_cmd(uint8_t cmd);
bool flash_erase(void);
bool write_memory(uint32_t addr, uint8_t *bytes, uint16_t n_bytes);
bool check_compatibility(void);

/* http.ino */
void create_response(Response *response);
void free_request(Request *request);
void free_response(Response *response);
void set_status(Response *response, int status_code);
bool add_content_headers(Response *response, int content_length, char *content_type);
bool add_header(Response *response, char *key, char *value);
bool add_header(Request *request, char *key, char *value);
int find_header(Request *request, char *header);
bool wait_on_client(WiFiClient client);
bool read_line(WiFiClient client, char *line, uint32_t max_size);
bool parse_method(WiFiClient client, Request *request);
bool header_accepted(char *key);
bool parse_header(Request *request, char *line);
bool parse_headers(WiFiClient client, Request *request);
bool parse_content_length(char *str_length, uint32_t *length);
bool read_bytes(WiFiClient client, uint8_t *bytes, uint32_t len);
bool parse_content(WiFiClient client, Request *request);
bool parse_request(WiFiClient client, Request *request);

/* serve_get.ino */
int serve_get(Request *request, Response *response);
int serve_file(Response *response, char *path, char *content_type);

/* serve_post.ino */
int serve_post(Request *request, Response *response);
int update_file(Request *request, char *content_type, char *file_name);
bool find_effect(uint8_t *buf, uint8_t buf_size, uint8_t effect_id, uint8_t effect_version, uint32_t *effect_index, uint32_t *effect_size, bool version_sensitive);
int serve_load(Request *request, Response *response);
int serve_save(Request *request, Response *response);
int serve_uart(Request *request, Response *response);
int serve_flash(Request *request, Response *response);

/* string_util.ino */
void substr(char *src, char *dst, int start, int end);
bool find(char *haystack, char *needle, int i);
bool string_split(char *src, char **dst, char *split, int max_n_splits, int *n_splits);
void to_lowercase(char *str);

/* file_util.ino */
void read_file(File *file, uint8_t *buf, uint32_t size);
void write_file(File *file, uint8_t *buf, uint32_t size);
void write_content(char *name, Content *content);
uint8_t *read_content(char *name, uint32_t *size);

/* server.ino */
bool server_init(void);
bool wifi_connect(void);
void server_loop(void *params);
void serve_request(WiFiClient client, Request *request);
void send_response(WiFiClient client, Response *response);

/* display.ino */
void display_init(void);
void display_str(String text);

void setup() {
  flash_init();

  display_init();

  if (!server_init()) {
    display_str("Server init error");
  }
}

void loop() { }