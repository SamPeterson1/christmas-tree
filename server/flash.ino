const uint8_t required_cmds[9] = {
  FLASH_CMD_GET,
  FLASH_CMD_EXT_ERASE_MEMORY,
  FLASH_CMD_WRITE_MEMORY,
  FLASH_CMD_READ_MEMORY,
};

void flash_init(void) {
  pinMode(BOOT0_PIN, OUTPUT);
  pinMode(BOOT1_PIN, OUTPUT);
  pinMode(NRST_PIN, OUTPUT);

  digitalWrite(NRST_PIN, HIGH);

  Serial.begin(115200, SERIAL_8E1);
}

void enter_bootloader(void) {
  digitalWrite(BOOT0_PIN, HIGH);
  digitalWrite(BOOT1_PIN, LOW);

  delay(100);
  reset();
}

void exit_bootloader(void) {
  digitalWrite(BOOT0_PIN, LOW);
  digitalWrite(BOOT1_PIN, LOW);

  delay(100);
  reset();
}

void reset(void) {
  digitalWrite(NRST_PIN, LOW);
  delay(100);
  digitalWrite(NRST_PIN, HIGH);
  delay(100);

  while (Serial.available()) {
    Serial.read();
  }
}

bool flash_program(uint8_t *bytes, uint32_t n_bytes) {
  enter_bootloader();

  if (!init_bootloader()) {
    return false;
  }
  
  if (!check_compatibility()) {
    return false;
  }

  Serial.setTimeout(100000);
  if (!flash_erase()) {
    return false;
  }
  Serial.setTimeout(1000);

  int n_chunks = n_bytes / 256;

  for (int i = 0; i < n_chunks; i ++) {
    if (!write_memory(0x08000000UL + i * 256, bytes + i * 256, 256)) {
      return false;
    }
  }

  int leftover = n_bytes - n_chunks * 256;

  if (!write_memory(0x08000000UL + n_chunks * 256, bytes + n_chunks * 256, leftover)) {
    return false;
  }

  exit_bootloader();

  return true;
}

bool init_bootloader(void) {
  Serial.write(FLASH_BOOT_UART);
  
  if (!check_ack()) {
    return false;
  }

  return true;
}

bool check_ack(void) {
  uint8_t rx_byte;

  if (Serial.readBytes(&rx_byte, 1) != 1) {
    return false;
  }

  return rx_byte == FLASH_ACK;
}

bool write_cmd(uint8_t cmd) {
  Serial.write(cmd);
  Serial.write(0xFF ^ cmd);

  return check_ack();
}

bool flash_erase(void) {
  if (!write_cmd(FLASH_CMD_EXT_ERASE_MEMORY)) {
    return false;
  }

  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0x00);

  return check_ack();
}

bool write_memory(uint32_t addr, uint8_t *bytes, uint16_t n_bytes) {
  if (!write_cmd(FLASH_CMD_WRITE_MEMORY)) {
    return false;
  }

  uint8_t addr_buf[4] = {
    (uint8_t) ((addr >> 24) & 0xFF),
    (uint8_t) ((addr >> 16) & 0xFF),
    (uint8_t) ((addr >> 8) & 0xFF),
    (uint8_t) ((addr >> 0) & 0xFF)
  };

  uint8_t checksum = addr_buf[0] ^ addr_buf[1] ^ addr_buf[2] ^ addr_buf[3];

  Serial.write(addr_buf, 4);
  Serial.write(checksum);

  if (!check_ack()) {
    return false;
  }

  checksum = n_bytes - 1;

  for (int i = 0; i < n_bytes; i ++) {
    checksum ^= bytes[i];
  }

  Serial.write(n_bytes - 1);
  Serial.write(bytes, n_bytes);
  Serial.write(checksum);

  if (!check_ack()) {
    return false;
  }

  return true;
}

bool check_compatibility(void) {
  if (!write_cmd(FLASH_CMD_GET)) {
    return false;
  }

  uint8_t n_cmds;

  if (Serial.readBytes(&n_cmds, 1) != 1) {
    return false;
  }
  
  uint8_t version;

  if (Serial.readBytes(&version, 1 ) != 1) {
    return false;
  }

  uint8_t *cmd_list = (uint8_t *) malloc(n_cmds * sizeof(uint8_t));

  if (Serial.readBytes(cmd_list, n_cmds) != n_cmds) {
    return false;
  }

  bool has_compatibility = true;

  for (int i = 0; i < sizeof(required_cmds); i ++) {
    bool has_cmd = false;

    for (int j = 0; j < n_cmds; j ++) {
      if (cmd_list[j] == required_cmds[i]) {
        has_cmd = true;
        break;
      }
    }

    if (!has_cmd) {
      has_compatibility = false;
      break;
    }
  }

  free(cmd_list);

  if (!check_ack()) {
    return false;
  }

  return has_compatibility;
}