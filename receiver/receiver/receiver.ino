// =========================
// HC-12
// =========================
#define RXD2 16
#define TXD2 17
#define SET_PIN 5

// =========================
// Estrutura do pacote
// =========================
struct __attribute__((packed)) Packet {

  uint8_t start;

  uint8_t apiario;

  uint8_t caixa;

  int16_t temperatura;

  uint8_t sequencia;

  uint8_t crc;
};

// =========================
// CRC XOR
// =========================
uint8_t calcularCRC(Packet p) {

  uint8_t crc = 0;

  crc ^= p.start;

  crc ^= p.apiario;

  crc ^= p.caixa;

  crc ^= (p.temperatura >> 8);

  crc ^= (p.temperatura & 0xFF);

  crc ^= p.sequencia;

  return crc;
}

void setup() {

  pinMode(SET_PIN, OUTPUT);

  digitalWrite(SET_PIN, HIGH);

  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("=== RECEIVER ===");

  Serial.print("Tamanho pacote: ");

  Serial.println(sizeof(Packet));
}

void loop() {

  // =========================
  // Procura início do pacote
  // =========================
  if (Serial2.available()) {

    uint8_t inicio = Serial2.read();

    // sincroniza
    if (inicio == 0xAA) {

      Packet p;

      p.start = inicio;

      uint8_t* ptr = (uint8_t*)&p;

      ptr++;

      int restante = sizeof(Packet) - 1;

      int lidos = Serial2.readBytes((char*)ptr, restante);

      if (lidos == restante) {

        uint8_t crcCalculado = calcularCRC(p);

        // =========================
        // Valida CRC
        // =========================
        if (p.crc == crcCalculado) {

          float temperatura = p.temperatura / 10.0;

          Serial.println();
          Serial.println("====== PACOTE ======");

          Serial.print("Apiario: ");

          Serial.println(p.apiario);

          Serial.print("Caixa: ");

          Serial.println(p.caixa);

          Serial.print("Temperatura: ");

          Serial.println(temperatura);

          Serial.print("Sequencia: ");

          Serial.println(p.sequencia);

          Serial.println("====================");
        }
        else {

          Serial.println("CRC inválido");
        }
      }
    }
  }
}
