#include "DHT.h"

// --- MAPEAMENTO FÍSICO ---
#define DHTPIN 15     // Pino de dados do sensor
#define DHTTYPE DHT11 // Sensor Azul
const int led1 = 12;  // Seu LED 1
const int led2 = 26;  // Seu LED 2
const int led3 = 32;  // Seu LED 3
const int ledInterno = 2;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Ajuste o Monitor Serial para 115200 baud para ler as mensagens
  Serial.begin(115200);
  Serial.println(F("--- Teste de Hardware Iniciado ---"));

  // Configuração dos pinos
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(ledInterno, OUTPUT);
  pinMode(DHTPIN, INPUT_PULLUP);

  // SEQUÊNCIA DE TESTE: Liga cada LED por 500ms para validar a solda
  Serial.println(F("Testando LEDs..."));
  digitalWrite(led1, HIGH); delay(500); digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH); delay(500); digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH); delay(500); digitalWrite(led3, LOW);
  digitalWrite(ledInterno, HIGH); delay(500); digitalWrite(ledInterno, LOW);

  dht.begin();
  Serial.println(F("Aguardando estabilização do sensor (2 segundos)..."));
  delay(2000);
}

void loop() {
  // Leitura do sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Verifica se a leitura falhou
  if (isnan(h) || isnan(t)) {
    Serial.println(F("ERRO: Falha na leitura do sensor! Verifique a porta 15 e a fiação."));
    
    // Pisca o LED interno em caso de erro no sensor
    digitalWrite(ledInterno, HIGH);
    delay(200);
    digitalWrite(ledInterno, LOW);
  } 
  else {
    // Exibe os dados limpos no Monitor Serial
    Serial.print(F("Umidade: "));
    Serial.print(h);
    Serial.print(F("% | Temperatura: "));
    Serial.print(t);
    Serial.println(F("°C"));

    // LÓGICA FÍSICA SIMPLES:
    // Se a temperatura for maior que 30°C, liga o LED 1
    if (t > 30.0) {
      digitalWrite(led1, HIGH);
    } else {
      digitalWrite(led1, LOW);
    }
  }

  // Espera 2 segundos para a próxima leitura (limite do DHT11)
  delay(2000);
}