#include <WiFi.h>        // Biblioteca para conectar o ESP32 ao Wi-Fi
#include <DHT.h>         // Biblioteca para ler o sensor DHT11
#include <WebServer.h>   // Biblioteca para criar um servidor web HTTP
#include <SPIFFS.h>      // Biblioteca para acessar arquivos salvos na memoria flash

// Definição dos pinos conectados aos LEDs
#define LED1 12
#define LED2 18
#define LED3 32

// Definição do pino e do tipo do sensor DHT
#define DHT_PIN 15
#define DHT_TYPE DHT11

// Variáveis usadas para fazer o LED piscar
unsigned long anteriorMillis = 0;   // Guarda o instante da ultima troca do pisca
const long intervalo = 500;         // Intervalo do pisca em ms
bool estadoPisca = LOW;             // Estado atual do pisca: LOW ou HIGH

// Credenciais da rede Wi-Fi
const char* ssid = "WifiDoZinho";
const char* password = "Zinho123";

// Cria o objeto do sensor DHT informando pino e tipo
DHT dht(DHT_PIN, DHT_TYPE);

// Cria o servidor web na porta 80, padrão HTTP
WebServer server(80);

// Variavel que vai guardar o modo atual do sistema, 0 - desligado, 1 - automatico, 2 - ligado
int modoSistema = 0;

// Temperatura limite para ativar alerta visual, ligar os 3 leds
const float LIMITE_TEMP = 30.0;

// Variáveis de controle de tempo
unsigned long tempoUltimoLog = 0;       // Momento da última gravação no log
unsigned long ultimaLeituraSensor = 0;  // Momento da última leitura do sensor

// Guardam a última leitura válida do sensor
float temperaturaAtual = NAN;
float umidadeAtual = NAN;


// Atualiza LEDs conforme o modo atual selecionado
void atualizarLEDs() {
  unsigned long atualMillis = millis(); 

  // Atualiza o estado do pisca a cada "intervalo" milissegundos
  if (atualMillis - anteriorMillis >= intervalo) {
    anteriorMillis = atualMillis;
    estadoPisca = !estadoPisca; // Inverte: se era 0 vira 1, se era 1 vira 0
  }
  
  // Modo 0 - Desligado: acende apenas o LED1  
  if (modoSistema == 0) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  } 
 // Modo 1 - Automático: LED1 e LED2 piscam juntos
 else if (modoSistema == 1) {
    digitalWrite(LED1, estadoPisca);
    digitalWrite(LED2, estadoPisca);
    digitalWrite(LED3, LOW);
  }
  // Modo 2: todos os LEDs acesos
  else if (modoSistema == 2) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
  } 
// Qualquer valor contrário: desliga tudo por "segurança"
else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
}

// Atualiza a leitura do sensor
void atualizarLeituraSensor() {
  if (millis() - ultimaLeituraSensor >= 2000) {
    ultimaLeituraSensor = millis();

    // Lê temperatura e umidade do sensor
    float t = dht.readTemperature();
    float u = dht.readHumidity();

    // Só atualiza as variáveis globais se as leituras forem válidas
    if (!isnan(t) && !isnan(u)) {
      temperaturaAtual = t;
      umidadeAtual = u;
    }

    // Exibe os valores no monitor serial para depuração
    Serial.print("Temperatura: ");
    Serial.println(temperaturaAtual);
    Serial.print("Umidade: ");
    Serial.println(umidadeAtual);
  }
}

// Essa secção do log foi com ajuda do gemini

// Salva leituras no arquivo de log
void salvarNoLog(float t, float u) {
  // FILE_APPEND abre o arquivo e escreve no final sem apagar o conteudo anterior
  File logFile = SPIFFS.open("/log.txt", FILE_APPEND);

  if (logFile) {
    // Salva os valores com duas casas decimais no formato: temperatura,umidade
    logFile.printf("%.2f,%.2f\n", t, u);
    logFile.close(); // Fecha o arquivo para garantir a gravacao
  } else {
    Serial.println("Erro ao abrir /log.txt para escrita");
  }
}

// Envia o arquivo index.html salvo no SPIFFS, envia arquivo direto pro navegador. Acessar utilizando o IP do ESP32 no navegador
void handleRoot() {
  if (SPIFFS.exists("/index.html")) {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "Arquivo index.html nao encontrado");
  }
}

// Envia os dados atuais do sensor em formato JSON para o navegador
void handleRead() {
  atualizarLeituraSensor(); // Garante que os dados não estejam muito atrasados em relação ao momento da requisição

  bool alertaAtivo = false;

  // Só ativa o alerta se a temperatura atual for válida
  if (!isnan(temperaturaAtual)) {
    alertaAtivo = (temperaturaAtual > LIMITE_TEMP);
  }

  // Monta manualmente um JSON
  String json = "{\"temp\":";
  json += isnan(temperaturaAtual) ? "\"--\"" : String(temperaturaAtual, 1);
  json += ",\"humi\":";
  json += isnan(umidadeAtual) ? "\"--\"" : String(umidadeAtual, 1);
  json += ",\"alerta\":";
  json += alertaAtivo ? "true" : "false";
  json += "}";

  // Envia a resposta HTTP com status 200 e tipo application/json, 200 significa Ok
  server.send(200, "application/json", json);
}

// Recebe o modo enviado pelo navegador e atualiza o sistema
void handleUpdate() {
  // Verifica se a URL trouxe o parametro "state"
  if (!server.hasArg("state")) {
    server.send(400, "text/plain", "Parametro state ausente");
    return;
  }

  // Converte o valor recebido para inteiro
  int novoModo = server.arg("state").toInt();

  // Garante que o modo recebido esteja entre 0 e 2
  if (novoModo < 0 || novoModo > 2) {
    server.send(400, "text/plain", "Modo inválido. Use 0, 1 ou 2");
    return;
  }

  // Atualiza o modo do sistema
  modoSistema = novoModo;

  // Atualiza imediatamente os LEDs
  atualizarLEDs();

  // Mostra o novo modo no monitor serial
  Serial.print("Modo atualizado para: ");
  Serial.println(modoSistema);

  // Informa ao navegador que deu certo
  server.send(200, "text/plain", "OK");
}

// Permite baixar o arquivo de log contendo as últimas medições, acessível através do IP do ESP32 seguido de /download no navegador
void handleDownload() {
  if (SPIFFS.exists("/log.txt")) {
    File file = SPIFFS.open("/log.txt", "r");

    // Faz o navegador entender que deve baixar o arquivo
    server.sendHeader("Content-Disposition", "attachment; filename=\"log.txt\"");
    server.streamFile(file, "text/plain");
    file.close();
  } else {
    server.send(404, "text/plain", "Log vazio");
  }
}

// Função executada uma vez ao ligar o ESP32
void setup() {
  Serial.begin(115200); // Inicia o monitor serial

  // Configura os pinos dos LEDs como saida
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  dht.begin(); // Inicializa o sensor DHT

  // Monta o sistema de arquivos SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS");
  }

  // Inicia a conexao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");

  // Espera ate conectar na rede
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Exibe informações da conexão no monitor serial
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Registra as rotas do servidor
  server.on("/", handleRoot);
  server.on("/read", handleRead);
  server.on("/update", handleUpdate);
  server.on("/download", handleDownload);

  server.begin();   // Inicia o servidor web
  atualizarLEDs();  // Atualiza o estado inicial dos LEDs
}

// Rotina LOOP, executada repetidamente 
void loop() {
  server.handleClient();    // Atende requisições HTTP do navegador
  atualizarLeituraSensor(); // Mantém os dados do sensor atualizados
  atualizarLEDs();          // Mantém o pisca pisca do modo automático funcionando

  // A cada 2 minutos, salva uma leitura no log se o sistema não estiver no modo desligado (modo 0)
  if (modoSistema != 0 && (millis() - tempoUltimoLog >= 120000)) {
    // Só grava se houver leitura valida
    if (!isnan(temperaturaAtual) && !isnan(umidadeAtual)) {
      salvarNoLog(temperaturaAtual, umidadeAtual);
      tempoUltimoLog = millis();
      Serial.println("Leitura salva no log");
    }
  }

  delay(10);
}