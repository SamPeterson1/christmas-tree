/* Read size bytes of a file into buf */
void read_file(File *file, uint8_t *buf, uint32_t size) {
  int n_blocks = size / 512;
  uint32_t offset = 0;

  for (int i = 0; i < n_blocks; i ++) {
    file->read(buf + offset, 512);
    offset += 512;
  }

  file->read(buf + offset, size - 512 * n_blocks);
}

/* Write size bytes of buf into a file */
void write_file(File *file, uint8_t *buf, uint32_t size) {
  int n_blocks = size / 512;
  uint32_t offset = 0;

  for (int i = 0; i < n_blocks; i ++) {
    file->write(buf + offset, 512);
    offset += 512;
  }

  file->write(buf + offset, size - 512 * n_blocks);
}

/* Write bytes of Content to a file */
void write_content(char *name, Content *content) {
  File file = SPIFFS.open(name, FILE_WRITE);
  
  write_file(&file, (uint8_t *) content->bytes, content->len);

  file.close();
}

/* Read all bytes of a file into a malloc'd buffer */
uint8_t *read_content(char *name, uint32_t *size) {  
  File file = SPIFFS.open(name, FILE_READ);
  Serial.flush();

  *size = file.size();
  uint8_t *buf = (uint8_t *) malloc(*size);
  read_file(&file, buf, *size);
  file.close();

  return buf;
}