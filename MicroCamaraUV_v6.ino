
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET 4
#define ONE_WIRE_BUS 2

// Constantes do display e da aquisição
const int pinoUV = A0;
const int Ndados = 1;
const int NleiturasUV = 100;
const int rele = 12;

const unsigned long intervaloLeitura = 15000UL; // intervalo entre leituras dos sensores (ms)
const unsigned long intervaloTela = 4000UL;     // intervalo de troca das telas (ms)
const unsigned long tempoEstabilizacaoUV = 15000UL; // espera após ligar a lâmpada (ms)

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperatura = DEVICE_DISCONNECTED_C;
float tensaoUV = 0.0;
float IntegralTensaoUV = 0.0; // Integral acumulada da tensão: soma de V*dt, em V.s
float Tmin = 0.0, Tmax = 0.0;
float Vmin = 0.0, Vmax = 0.0;
bool temperaturaValida = false;
String Message = "Designed by LABSOLAR";

unsigned long ultimaTroca = 0;
unsigned long ultimaLeitura = 0;
unsigned long ultimaIntegracaoUV = 0;
int paginaAtual = 0; // 0 = temperatura, 1 = tensão UV, 2 = integral UV
const int numeroTelas = 3;

float lerTemperaturaMedia();
float lerTensaoUV();
void acumularIntegralUV();
void desenharTela();
void show();

void setup() {
  Serial.begin(9600);

  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Falha ao iniciar o display SSD1306"));
    for (;;) {}
  }

  show(); // Exibe a tela inicial
  sensors.begin();
  delay(2000);
  digitalWrite(rele, HIGH);

  // Aguarda a fonte UV estabilizar antes de considerar a primeira medição.
  // Ajuste tempoEstabilizacaoUV conforme o comportamento da sua lâmpada.
  delay(tempoEstabilizacaoUV);

  temperatura = lerTemperaturaMedia();
  temperaturaValida = (temperatura != DEVICE_DISCONNECTED_C);
  if (temperaturaValida) {
    Tmin = temperatura;
    Tmax = temperatura;
  }

  tensaoUV = lerTensaoUV();
  Vmin = tensaoUV;
  Vmax = tensaoUV;
  IntegralTensaoUV = 0.0;

  desenharTela();

  // Inicia os temporizadores após a primeira leitura.
  //unsigned long agora = millis();
  ultimaTroca = millis();
  ultimaLeitura = millis();
  ultimaIntegracaoUV = millis();
}

void loop() {
  // Integra a tensão medida ao longo do tempo usando o valor mais recente.
  // O resultado tem unidade V.s e ainda não é uma energia em J/m^2.
  acumularIntegralUV();

  //unsigned long agora = millis();
  bool atualizarTela = false;

  // Faz novas leituras dos sensores a cada intervaloLeitura.
  if (millis() - ultimaLeitura >= intervaloLeitura) {
    ultimaLeitura = millis();

    temperatura = lerTemperaturaMedia();
    temperaturaValida = (temperatura != DEVICE_DISCONNECTED_C);

    if (temperaturaValida) {
      if (temperatura < Tmin) Tmin = temperatura;
      if (temperatura > Tmax) Tmax = temperatura;
    }

    float novaTensaoUV = lerTensaoUV();

    // Inclui o tempo gasto nas leituras usando a tensão anterior, antes de
    // substituir pelo novo valor medido.
    acumularIntegralUV();
    tensaoUV = novaTensaoUV;

    if (tensaoUV < Vmin) Vmin = tensaoUV;
    if (tensaoUV > Vmax) Vmax = tensaoUV;

    Serial.print(F("Temperatura: "));
    if (temperaturaValida) {
      Serial.print(temperatura, 1);
      Serial.print(F(" C"));
    } else {
      Serial.print(F("Erro no sensor"));
    }

    Serial.print(F(" | Tensao media do sensor UV: "));
    Serial.print(tensaoUV, 3);
    Serial.print(F(" V | Integral V.t: "));
    Serial.print(IntegralTensaoUV, 2);
    Serial.println(F(" V.s"));

    atualizarTela = true;
  }

  // Alterna entre três páginas sem bloquear as leituras:
  // 0 = temperatura, 1 = tensão UV, 2 = integral acumulada V.s.
  //agora = millis();
  if (millis() - ultimaTroca >= intervaloTela) {
    paginaAtual = (paginaAtual + 1) % numeroTelas;
    ultimaTroca = millis();
    atualizarTela = true;
  }

  if (atualizarTela) {
    desenharTela();
  }

  delay(5); // Evita um loop desnecessariamente apertado
}

float lerTemperaturaMedia() {
  float soma = 0.0;

  for (int i = 0; i < Ndados; i++) {
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);

    if (t == DEVICE_DISCONNECTED_C || t == 85.0 || t < -55.0 || t > 125.0) {
      return DEVICE_DISCONNECTED_C;
    }

    soma += t;
    if (i < Ndados - 1) delay(200);
  }

  return soma / Ndados;
}

float lerTensaoUV() {
  unsigned long somaLeituras = 0;

  for (int i = 0; i < NleiturasUV; i++) {
    somaLeituras += analogRead(pinoUV);
    delay(2);
  }

  float leituraMedia = (float)somaLeituras / NleiturasUV;
  // Aproximação para Arduino Nano clássico usando referência de 5 V.
  return leituraMedia * (5.0 / 1023.0);
}

void acumularIntegralUV() {
 // unsigned long agora = millis();
  unsigned long deltaMs = millis() - ultimaIntegracaoUV;
  IntegralTensaoUV += tensaoUV * (deltaMs / 1000.0);
  ultimaIntegracaoUV = millis();
}

void desenharTela() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  if (paginaAtual == 0) {
    // Página 1: temperatura
    display.setCursor(5, 0);
    display.println(F(">>> Temperatura <<<"));

    display.setTextSize(2);
    display.setCursor(12, 18);

    if (!temperaturaValida) {
      display.println(F("Erro sensor"));
    } else {
      display.print(temperatura, 1);
      display.println(F(" C"));
    }

    display.setTextSize(1);
    display.setCursor(2, 53);
    display.print(F("Min "));
    if (temperaturaValida) display.print(Tmin, 1);
    else display.print(F("--"));
    display.print(F(" Max "));
    if (temperaturaValida) display.print(Tmax, 1);
    else display.print(F("--"));

  } else if (paginaAtual == 1) {
    // Página 2: tensão instantânea do sensor UV
    display.setCursor(5, 0);
    display.println(F(">>> Sensor UV <<<"));

    display.setTextSize(2);
    display.setCursor(8, 20);
    display.print(tensaoUV, 3);
    display.println(F(" V"));

    display.setTextSize(1);
    display.setCursor(2, 53);
    display.print(F("Min "));
    display.print(Vmin, 3);
    display.print(F(" Max "));
    display.print(Vmax, 3);

  } else {
    // Página 3: integral temporal da tensão, separada da leitura de tensão.
    display.setCursor(5, 0);
    display.println(F(">> Int. tensao <<"));

    display.setCursor(2, 17);
    display.println(F("Tensao x tempo:"));

    display.setTextSize(2);
    display.setCursor(2, 30);
    display.print(IntegralTensaoUV, 2);

    display.setTextSize(1);
    display.setCursor(2, 54);
    display.print(F("Unidade: V.s"));
  }

  display.display();
}

//----------------------//
// >>>>> SUBROUTINES <<<<<
//----------------------//
void show() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 1);
  display.println(F("<<<< ---------- >>>>"));
  display.setCursor(15, 16);
  display.println(F("   Thermometre"));
  display.setCursor(15, 28);
  display.println(F("   Sensor UV"));
  display.setCursor(15, 40);
  display.println(F("   versao 1.2"));
  display.setCursor(2, 54);
  display.print(Message);
  display.display();
  delay(4000);
}
