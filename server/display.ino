SSD1306Wire displ(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void display_init(void) {
  displ.init();
  displ.setFont(ArialMT_Plain_10);
  displ.setTextAlignment(TEXT_ALIGN_LEFT);
}

void display_str(String text) {
  displ.clear();
  displ.drawStringMaxWidth(0, 0, 128, text);
  displ.display();
}