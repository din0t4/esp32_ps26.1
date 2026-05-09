// =============================================
//  Firmware Sincronizado com index.html
// =============================================

#include <WiFi.h>
#include <DHT.h>
#include <WebServer.h>
#include <SPIFFS.h>

#define LED1 12
#define LED2 18 
#define LED3 32
#define DHT_PIN 15
#define DHT_TYPE DHT11

const char* ssid = "WifiDoZinho";
const char* password = "Zinho123";

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

// Variável para armazenar o modo (0=Desligado, 1=Automático, 2=Ligado)
int modoSistema = 0;

void handleRoot() {
  if (SPIFFS.exists("/index.html")) {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "Arquivo index.html nao encontrado no SPIFFS");
  }
}

// Rota /read (Sincronizada com o JavaScript do seu HTML)
void handleRead() {
  float t = dht.readTemperature();
  float u = dht.readHumidity();
  
  // Cria o JSON com os nomes de campos que o seu JS espera (temp e humi)
  String json = "{\"temp\":";
  json += (isnan(t)) ? "\"--\"" : String(t);
  json += ",\"humi\":";
  json += (isnan(u)) ? "\"--\"" : String(u);
  json += "}";
  
  server.send(200, "application/json", json);
}

// Rota /update (Sincronizada com a funcao mudarModo do seu HTML)
void handleUpdate() {
  if (server.hasArg("state")) {
    modoSistema = server.arg("state").toInt();
    Serial.print("Novo modo recebido: ");
    Serial.println(modoSistema);
    server.send(200, "text/plain", "OK");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  dht.begin();
  if (!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Definição das rotas conforme o seu arquivo index.html
  server.on("/", handleRoot);
  server.on("/read", handleRead);       // JS: fetch('/read')
  server.on("/update", handleUpdate);   // JS: fetch('/update?state=...')
  
  server.begin();

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("ESP32 iniciada e pronta!");
}

void loop() {
  server.handleClient();

  // Exemplo de lógica baseada no modo selecionado no HTML
  if (modoSistema > 0) { // Se não estiver "Desligado"
    // Pisca LEDs ou executa automação
    digitalWrite(LED1, HIGH); delay(100); digitalWrite(LED1, LOW);
  }

  // Pequeno delay para estabilidade
  delay(10); 
}