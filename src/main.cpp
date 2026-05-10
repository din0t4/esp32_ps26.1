#include <WiFi.h>      // Biblioteca para conectar o ESP32 ao Wi-Fi
#include <DHT.h>       // Biblioteca para ler o sensor DHT11
#include <WebServer.h> // Biblioteca para criar um servidor web HTTP
#include <SPIFFS.h>    // Biblioteca para acessar arquivos salvos na memoria flash
#include <time.h>      // Biblioteca para obter data e hora via NTP
// Definição dos pinos conectados aos LEDs
#define LED1 12
#define LED2 26
#define LED3 32

// Definição do pino e do tipo do sensor DHT
#define DHT_PIN 15
#define DHT_TYPE DHT11

// Variáveis usadas para fazer o LED piscar
unsigned long anteriorMillis = 0; // Guarda o instante da ultima troca do pisca
const long intervalo = 500;       // Intervalo do pisca em ms
bool estadoPisca = LOW;           // Estado atual do pisca: LOW ou HIGH

// Credenciais da rede Wi-Fi
const char *ssid = "WifiDoZinho";
const char *password = "Zinho123";

// Cria o objeto do sensor DHT informando pino e tipo
DHT dht(DHT_PIN, DHT_TYPE);

// Cria o servidor web na porta 80, padrão HTTP
WebServer server(80);

// Variavel que vai guardar o modo atual do sistema, 0 - desligado, 1 - automatico, 2 - ligado
int modoSistema = 0;

// Temperatura limite para ativar alerta visual, ligar os 3 leds
float LIMITE_TEMP = 30.0;
const float LIMITE_UMIDADE = 30.0;

// Variável para controlar se a temperatura deve ser exibida em Fahrenheit ou Celsius
bool tempFahrenheit = false;

// Variáveis de controle de tempo
unsigned long tempoUltimoLog = 0;      // Momento da última gravação no log
unsigned long ultimaLeituraSensor = 0; // Momento da última leitura do sensor

// Guardam a última leitura válida do sensor
float temperaturaAtualC = NAN;
float umidadeAtual = NAN;

// Indica se o sensor está conectado
bool sensorConectado = false;

// Conversão de Farenheit para Celsius

float celsiusParaFahrenheit(float valorC)
{
  return (valorC * 9.0 / 5.0) + 32.0;
}

// Conversão de Celsius para Farenheit

float fahrenheitParaCelsius(float valorF)
{
  return (valorF - 32.0) * 5.0 / 9.0;
}
float UnidadeAtual(float valorC)
{
  return tempFahrenheit ? celsiusParaFahrenheit(valorC) : valorC;
}

// Formata a temperatura para exibição, considerando a unidade atual e tratando valores inválidos

String textoTemperatura(float valorC)
{
  if (isnan(valorC))
  {
    return "\"--\"";
  }

  return String(UnidadeAtual(valorC), 1);
}

// Formata a umidade para exibição, tratando valores inválidos

String textoUmidade(float valor)
{
  if (isnan(valor))
  {
    return "\"--\"";
  }
  return String(valor, 1);
}

// Verifica se o alerta de temperatura deve estar ativo

bool alertaTemperaturaAtivo()
{
  return sensorConectado && !isnan(temperaturaAtualC) && temperaturaAtualC >= LIMITE_TEMP;
}

// Verifica se o alerta de umidade deve estar ativo

bool alertaUmidadeAtivo()
{
  return sensorConectado && !isnan(umidadeAtual) && umidadeAtual <= LIMITE_UMIDADE;
}

// Marca o sensor como desconectado, limpando as leituras atuais

void marcarSensorDesconectado()
{
  sensorConectado = false;
  temperaturaAtualC = NAN;
  umidadeAtual = NAN;
}

// Lê o sensor DHT e atualiza as variáveis globais, retornando true se a leitura foi válida ou false se houve erro

bool lerSensorAgora()
{
  float t = dht.readTemperature();
  float u = dht.readHumidity();

  if (isnan(t) || isnan(u))
  {
    marcarSensorDesconectado();
    Serial.println("Sensor DHT desconectado");
    return false;
  }

  sensorConectado = true;
  temperaturaAtualC = t;
  umidadeAtual = u;

  Serial.print("Temperatura: ");
  Serial.println(temperaturaAtualC);
  Serial.print("Umidade: ");
  Serial.println(umidadeAtual);
  return true;
}

// Atualiza LEDs conforme o modo atual selecionado
void atualizarLEDs()
{
  unsigned long atualMillis = millis();

  bool alertaAtivo = alertaTemperaturaAtivo() || alertaUmidadeAtivo();

  // Se houver alerta, os 3 LEDs piscam rapidamente
  if (alertaAtivo)
  {
    if (atualMillis - anteriorMillis >= 100)
    {
      anteriorMillis = atualMillis;
      estadoPisca = !estadoPisca;
    }

    digitalWrite(LED1, estadoPisca);
    digitalWrite(LED2, estadoPisca);
    digitalWrite(LED3, estadoPisca);
    return;
  }

  // Atualiza o estado do pisca a cada "intervalo" milissegundos
  if (atualMillis - anteriorMillis >= intervalo)
  {
    anteriorMillis = atualMillis;
    estadoPisca = !estadoPisca; // Inverte: se era 0 vira 1, se era 1 vira 0
  }

  // Modo 0 - Desligado: acende apenas o LED1
  if (modoSistema == 0)
  {
    digitalWrite(LED1, estadoPisca);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
  // Modo 1 - Automático: LED1 e LED2 piscam juntos
  else if (modoSistema == 1)
  {
    digitalWrite(LED1, estadoPisca);
    digitalWrite(LED2, estadoPisca);
    digitalWrite(LED3, LOW);
  }
  // Modo 2: todos os LEDs acesos
  else if (modoSistema == 2)
  {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
  }
  // Qualquer valor contrário: desliga tudo por "segurança"
  else
  {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
}

// Atualiza a leitura do sensor se o intervalo desde a última leitura for maior que o definido para o modo atual
void atualizarLeituraSensor()
{

  // Modo desligado: limpa os dados e não realiza leituras
  if (modoSistema == 0)
  {
    marcarSensorDesconectado();
    return;
  }

  unsigned long intervaloLeitura = (modoSistema == 0) ? 2000 : 1000;

  if (millis() - ultimaLeituraSensor >= intervaloLeitura)
  {
    ultimaLeituraSensor = millis();
    lerSensorAgora();
  }
}

// Essa secção do log foi com ajuda do gemini
// Salva leituras no arquivo de log
void salvarNoLog(float tC, float u)
{
  // 1. Obter o tempo atual
  time_t now = time(nullptr);
  struct tm *t_info = localtime(&now);

  // 2. Abrir o arquivo no modo "append" (adicionar ao final)
  File logFile = SPIFFS.open("/log.txt", "a");

  if (logFile)
  {
    // 3. Criamos as variáveis necessárias para a exibição correta no log
    float t = UnidadeAtual(tC);
    const char *unidade = tempFahrenheit ? "F" : "C";

    logFile.printf("Dia: %02d/%02d/%02d - Hora: %02d/%02d:%02d Temperatura: %.2f %s - Umidade: %.2f %%\n",
                   t_info->tm_mday,
                   t_info->tm_mon + 1,
                   (t_info->tm_year + 1900) % 100,
                   t_info->tm_hour,
                   t_info->tm_min,
                   t_info->tm_sec,
                   t,
                   unidade,
                   u);

    logFile.close();
  }
  else
  {
    Serial.println("Erro ao abrir o arquivo de log para escrita");
  }
}

// Envia o arquivo index.html
void handleRoot()
{
  if (SPIFFS.exists("/index.html"))
  {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  }
  else
  {
    server.send(404, "text/plain", "Arquivo index.html nao encontrado");
  }
}

// Envia os dados atuais do sensor em formato JSON para o navegador
void handleRead()
{
  atualizarLeituraSensor();

  // Modo desligado: retorna apenas o modo, sem dados de sensor
  if (modoSistema == 0)
  {
    server.send(200, "application/json", "{\"modo\":0,\"desligado\":true}");
    return;
  }

  // Prepara a resposta JSON com os dados atuais do sensor e o estado do sistema
  String json = "{\"temp\":";
  json += textoTemperatura(temperaturaAtualC);
  json += ",\"humi\":";
  json += textoUmidade(umidadeAtual);
  json += ",\"alertaTemp\":";
  json += alertaTemperaturaAtivo() ? "true" : "false";
  json += ",\"alertaHumi\":";
  json += alertaUmidadeAtivo() ? "true" : "false";
  json += ",\"sensorOk\":";
  json += sensorConectado ? "true" : "false";
  json += ",\"modo\":";
  json += String(modoSistema);
  json += ",\"unit\":\"";
  json += tempFahrenheit ? "F" : "C";
  json += "\",\"tempMax\":";
  json += String(UnidadeAtual(LIMITE_TEMP), 1);
  json += "}";

  // Envia a resposta HTTP com status 200, 200 significa Ok
  server.send(200, "application/json", json);
}

// Recebe o modo enviado pelo navegador e atualiza o sistema
void handleUpdate()
{
  // Verifica se a URL trouxe o parametro "state"
  if (!server.hasArg("state"))
  {
    server.send(400, "text/plain", "Parametro state ausente");
    return;
  }

  // Converte o valor recebido para inteiro
  int novoModo = server.arg("state").toInt();

  // Garante que o modo recebido esteja entre 0 e 2
  if (novoModo < 0 || novoModo > 2)
  {
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

void handleConfig()
{
  if (modoSistema != 2)
  {
    server.send(409, "text/plain", "Configuração permitida apenas no modo Ligado");
    return;
  }

  if (server.hasArg("unit"))
  {
    String unit = server.arg("unit");

    if (unit == "C")
    {
      tempFahrenheit = false;
    }
    else if (unit == "F")
    {
      tempFahrenheit = true;
    }
    else
    {
      server.send(400, "text/plain", "Unidade inválida. Use C ou F");
      return;
    }
  }

  if (server.hasArg("tempMax"))
  {
    float novoLimite = server.arg("tempMax").toFloat();
    // Garante a unidade padrão Celsius internamente
    LIMITE_TEMP = tempFahrenheit ? fahrenheitParaCelsius(novoLimite) : novoLimite;
  }

  handleRead();
}

// Permite baixar o arquivo de log contendo as últimas medições, acessível através do IP do ESP32 seguido de /download no navegador
void handleDownload()
{
  if (SPIFFS.exists("/log.txt"))
  {
    File file = SPIFFS.open("/log.txt", "r");

    // Faz o navegador entender que deve baixar o arquivo
    server.sendHeader("Content-Disposition", "attachment; filename=\"log.txt\"");
    server.streamFile(file, "text/plain");
    file.close();
  }
  else
  {
    server.send(404, "text/plain", "Log vazio");
  }
}

// Função executada uma vez ao ligar o ESP32
void setup()
{
  Serial.begin(115200); // Inicia o monitor serial

  // Configura os pinos dos LEDs como saída
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  dht.begin(); // Inicializa o sensor DHT

  // Monta o sistema de arquivos SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Erro ao montar SPIFFS");
  }

  // Inicia a conexão Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");

  // Espera até conectar na rede
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Exibe informações da conexão no monitor serial
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Sincroniza o relógio via NTP (UTC-3 = horário de Brasília)
  configTime(-3 * 3600, 0, "pool.ntp.org");
  Serial.println("Sincronizando horário NTP...");

  // Registra as rotas do servidor
  server.on("/", handleRoot);
  server.serveStatic("/css/", SPIFFS, "/css/");
  server.serveStatic("/js/", SPIFFS, "/js/");
  server.serveStatic("/images/", SPIFFS, "/images/");

  server.on("/read", handleRead);
  server.on("/update", handleUpdate);
  server.on("/config", handleConfig);
  server.on("/download", handleDownload);

  server.begin();  // Inicia o servidor web
  atualizarLEDs(); // Atualiza o estado inicial dos LEDs
}

void loop()
{
  server.handleClient();    // Atende requisições HTTP do navegador
  atualizarLeituraSensor(); // Mantém os dados do sensor atualizados
  atualizarLEDs();          // Mantém o pisca pisca do modo automático funcionando

  // No modo desligado, garantimos que os dados do sensor fiquem limpos e não sejam atualizados
  if (modoSistema == 0)
  {
    marcarSensorDesconectado(); // Garante que os dados fiquem limpos no modo desligado
  }
  else
  {
    atualizarLeituraSensor(); // Mantém os dados do sensor atualizados
  }
  
  // A cada 2 minutos, salva uma leitura no log se o sistema não estiver no modo desligado (modo 0)
  if (modoSistema == 1 && (millis() - tempoUltimoLog >= 12000))
  {
    // Só grava no log se o sensor estiver conectado e as leituras forem válidas
    if (sensorConectado && !isnan(temperaturaAtualC) && !isnan(umidadeAtual))
    {
      salvarNoLog(temperaturaAtualC, umidadeAtual);
      tempoUltimoLog = millis();
      Serial.println("Leitura salva no log");
    }
  }

  delay(10);
}
