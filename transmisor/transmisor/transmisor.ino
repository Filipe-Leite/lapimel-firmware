#include "DHT.h"

// =========================
// HC-12
// =========================
#define RXD2 16
#define TXD2 17
#define SET_PIN 5

// =========================
// DHT
// =========================
#define DHTPIN 19

// TROQUE PARA DHT11 SE NECESSÁRIO
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// =========================
// IDs
// =========================
#define APIARIO_ID 1
#define CAIXA_ID 12

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

uint8_t seq = 0;

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

  dht.begin();

  randomSeed(analogRead(34));

  Serial.println("=== TRANSMISSOR ===");

  Serial.print("Tamanho pacote: ");

  Serial.println(sizeof(Packet));
}

void loop() {

  float temp = dht.readTemperature();

  if (!isnan(temp)) {

    Packet p;

    p.start = 0xAA;

    p.apiario = APIARIO_ID;

    p.caixa = CAIXA_ID;

    // temperatura x10
    p.temperatura = (int16_t)(temp * 10);

    p.sequencia = seq++;

    p.crc = calcularCRC(p);

    // =========================
    // ENVIA PACOTE BINÁRIO
    // =========================
    Serial2.write((uint8_t*)&p, sizeof(p));

    Serial2.flush();

    Serial.println("Pacote enviado");

    Serial.print("Temperatura: ");

    Serial.println(temp);
  }
  else {

    Serial.println("Erro DHT");
  }


  // anti-colisão
  delay(random(500, 1500));
}
