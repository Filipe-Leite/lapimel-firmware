#include "DHT.h"

// =========================
// HC-12
// =========================
#define RXD2 16
#define TXD2 17
#define SET_PIN 5

// =========================
// DHT22
// =========================
#define DHTPIN 19
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// =========================
// BOTÃO
// =========================
#define BUTTON_PIN 15

// =========================
// DISPLAY 7 SEGMENTOS
// =========================

// SEGMENTOS
const int segA  = 13;
const int segB  = 12;
const int segC  = 14;
const int segD  = 27;
const int segE  = 26;
const int segF  = 25;
const int segG  = 33;
const int segDP = 32;

// DÍGITOS
const int dig1 = 4;
const int dig2 = 18;
const int dig3 = 23;

// =========================
// TABELA NÚMEROS
// =========================
byte numeros[10][7] = {

  {1,1,1,1,1,1,0}, //0
  {0,1,1,0,0,0,0}, //1
  {1,1,0,1,1,0,1}, //2
  {1,1,1,1,0,0,1}, //3
  {0,1,1,0,0,1,1}, //4
  {1,0,1,1,0,1,1}, //5
  {1,0,1,1,1,1,1}, //6
  {1,1,1,0,0,0,0}, //7
  {1,1,1,1,1,1,1}, //8
  {1,1,1,1,0,1,1}  //9
};

// =========================
// IDs
// =========================
#define APIARIO_ID 1
#define CAIXA_ID 12

// =========================
// PACOTE
// =========================
struct __attribute__((packed)) Packet {

  uint8_t start;

  uint8_t apiario;

  uint8_t caixa;

  int16_t temperatura;

  uint16_t umidade;

  uint8_t sequencia;

  uint8_t crc;
};

uint8_t seq = 0;

// =========================
// CONTROLE TEMPO
// =========================
unsigned long ultimoEnvio = 0;

const unsigned long intervaloEnvio = 1000;

// =========================
// DISPLAY TIMER
// =========================
bool displayLigado = false;

unsigned long tempoDisplay = 0;

const unsigned long tempoLigado = 5000;

// =========================
// TEMPERATURA GLOBAL
// =========================
float temperaturaAtual = 0;

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

  crc ^= (p.umidade >> 8);
  crc ^= (p.umidade & 0xFF);

  crc ^= p.sequencia;

  return crc;
}

// =========================
// DESLIGA DÍGITOS
// =========================
void desligarDigitos() {

  digitalWrite(dig1, HIGH);
  digitalWrite(dig2, HIGH);
  digitalWrite(dig3, HIGH);
}

// =========================
// APAGA SEGMENTOS
// =========================
void apagarSegmentos() {

  digitalWrite(segA, LOW);
  digitalWrite(segB, LOW);
  digitalWrite(segC, LOW);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, LOW);
  digitalWrite(segDP, LOW);
}

// =========================
// APAGA DISPLAY
// =========================
void apagarDisplay() {

  desligarDigitos();

  apagarSegmentos();
}

// =========================
// MOSTRA NÚMERO
// =========================
void mostrarNumero(int numero) {

  digitalWrite(segA, numeros[numero][0]);
  digitalWrite(segB, numeros[numero][1]);
  digitalWrite(segC, numeros[numero][2]);
  digitalWrite(segD, numeros[numero][3]);
  digitalWrite(segE, numeros[numero][4]);
  digitalWrite(segF, numeros[numero][5]);
  digitalWrite(segG, numeros[numero][6]);
}

// =========================
// MULTIPLEXAÇÃO
// =========================
void atualizarDisplay(float temp) {

  int valor = (int)(temp * 10);

  int d1 = valor / 100;
  int d2 = (valor / 10) % 10;
  int d3 = valor % 10;

  // DIGITO 1
  desligarDigitos();

  mostrarNumero(d1);

  digitalWrite(segDP, LOW);

  digitalWrite(dig1, LOW);

  delayMicroseconds(3000);

  // DIGITO 2
  desligarDigitos();

  mostrarNumero(d2);

  digitalWrite(segDP, HIGH);

  digitalWrite(dig2, LOW);

  delayMicroseconds(3000);

  // DIGITO 3
  desligarDigitos();

  mostrarNumero(d3);

  digitalWrite(segDP, LOW);

  digitalWrite(dig3, LOW);

  delayMicroseconds(3000);
}

void setup() {

  // HC-12
  pinMode(SET_PIN, OUTPUT);

  digitalWrite(SET_PIN, HIGH);

  // BOTÃO
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // SERIAL
  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // DHT
  dht.begin();

  // DISPLAY
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);
  pinMode(segDP, OUTPUT);

  pinMode(dig1, OUTPUT);
  pinMode(dig2, OUTPUT);
  pinMode(dig3, OUTPUT);

  apagarDisplay();

  Serial.println("Sistema iniciado");
}

void loop() {

  // =========================
  // BOTÃO
  // =========================
  if (digitalRead(BUTTON_PIN) == LOW) {

    displayLigado = true;

    tempoDisplay = millis();
  }

  // =========================
  // DISPLAY
  // =========================
  if (displayLigado) {

    atualizarDisplay(temperaturaAtual);

    if (millis() - tempoDisplay >= tempoLigado) {

      displayLigado = false;

      apagarDisplay();
    }
  }
  else {

    apagarDisplay();
  }

  // =========================
  // ENVIO HC-12
  // =========================
  if (millis() - ultimoEnvio >= intervaloEnvio) {

    ultimoEnvio = millis();

    float temp = dht.readTemperature();

    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {

      temperaturaAtual = temp;

      Packet p;

      p.start = 0xAA;

      p.apiario = APIARIO_ID;

      p.caixa = CAIXA_ID;

      p.temperatura = (int16_t)(temp * 10);

      p.umidade = (uint16_t)(hum * 10);

      p.sequencia = seq++;

      p.crc = calcularCRC(p);

      Serial2.write((uint8_t*)&p, sizeof(p));

      Serial2.flush();

      Serial.println("Pacote enviado");

      Serial.print("Temperatura: ");

      Serial.println(temp);

      Serial.print("Umidade: ");

      Serial.println(hum);
    }
  }
}
