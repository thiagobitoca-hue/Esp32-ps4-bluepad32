#include <Arduino.h>
#include <Bluepad32.h>

ControllerPtr controllers[BP32_MAX_CONTROLLERS];

void onConnectedController(ControllerPtr ctl) {
  Serial.println(">> Controle conectado!");
  ctl->setRumble(0x60, 0x60);
  delay(150);
  ctl->setRumble(0, 0);
  ctl->setColorLED(0, 0, 255); // Azul no PS4
}

void onDisconnectedController(ControllerPtr ctl) {
  Serial.println(">> Controle desconectado.");
  for (int i = 0; i < BP32_MAX_CONTROLLERS; ++i)
    if (controllers[i] == ctl) { controllers[i] = nullptr; break; }
}

void addController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; ++i)
    if (controllers[i] == nullptr) { controllers[i] = ctl; return; }
  Serial.println("Sem slots livres para novos controles!");
}

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println("\n=== Bluepad32 PS4 + ESP32 ===");
  Serial.println("Parear: PS + Share (LED piscando rapido)");
  // Se precisar limpar chaves BT antigas, descomente por UMA execucao:
  // BP32.forgetBluetoothKeys();

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableNewBluetoothConnections(true);
}

void printControllerData(ControllerPtr c) {
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last < 80) return;
  last = now;

  int32_t lx = c->axisX(), ly = c->axisY();
  int32_t rx = c->axisRX(), ry = c->axisRY();
  int32_t th = c->throttle(), br = c->brake();

  Serial.printf("LX:%4ld LY:%4ld RX:%4ld RY:%4ld Th:%4ld Br:%4ld  A:%d B:%d X:%d Y:%d L1:%d R1:%d PS:%d\n",
                lx, ly, rx, ry, th, br, c->a(), c->b(), c->x(), c->y(), c->l1(), c->r1(), c->meta());
}

void loop() {
  BP32.update();

  ControllerPtr nc;
  while ((nc = BP32.getController())) addController(nc);

  for (int i = 0; i < BP32_MAX_CONTROLLERS; ++i) {
    auto c = controllers[i];
    if (c && c->isConnected() && c->hasData()) {
      printControllerData(c);
      c->setRumble(c->l1() ? 0x40 : 0, c->l1() ? 0x40 : 0);
    }
  }
  delay(1);
}
