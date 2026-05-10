# 🌡️ Sistema de Monitoramento de Temperatura e Umidade (ESP32)

Este projeto consiste em um sistema de monitoramento ambiental em tempo real utilizando um microcontrolador **ESP32** e um sensor **DHT11**. O sistema oferece uma interface web interativa para visualização de dados, configuração de alertas e gerenciamento de logs históricos.

---

## 🚀 Funcionalidades

* **Monitoramento em Tempo Real:** Leitura precisa de temperatura e umidade.
* **Interface Web Responsiva:** Dashboard moderno (HTML/CSS) para controle e visualização via navegador.
* **Três Modos de Operação:**
    * 🔴 **Desligado:** Sistema em repouso (apenas LED1 pisca indicando atividade).
    * 🔵 **Automático:** Monitoramento ativo com gravação de logs no SPIFFS a cada 2 minutos.
    * 🟢 **Ligado:** Modo de monitoramento contínuo e permissão para alteração de configurações.
* **Alertas Visuais:** LEDs piscam rapidamente caso a temperatura ultrapasse o limite ou a umidade caia abaixo do nível de segurança.
* **Sistema de Log:** Armazenamento de dados na memória interna (SPIFFS) com timestamps sincronizados via NTP.
* **Configuração Dinâmica:** Alternância entre Celsius (°C) e Fahrenheit (°F) e ajuste de limites térmicos via interface.

---

## 🛠️ Hardware Necessário

| Componente | Especificação |
| :--- | :--- |
| **Microcontrolador** | ESP32 (WROOM-32) |
| **Sensor** | DHT11 (ou DHT22 com ajuste no código) |
| **Atuadores** | 3 LEDs (Resistores de 220Ω recomendados) |

### 📌 Pinagem (GPIOs)
* **GPIO 12:** LED 1 (Status / Modo Desligado)
* **GPIO 26:** LED 2 (Modo Automático / Alerta)
* **GPIO 32:** LED 3 (Modo Ligado / Alerta)
* **GPIO 15:** Sensor DHT11

---

## 📂 Estrutura de Arquivos (SPIFFS)

Os arquivos abaixo devem ser carregados na memória Flash do ESP32 para o funcionamento da interface web:

```text
/
├── index.html       # Página principal da interface
├── css/
│   └── styles.css   # Estilização do dashboard
├── js/
│   └── script.js    # Lógica de atualização e requisições AJAX
├── images/
│   └── logo.png     # Identidade visual (EletronJun)
└── log.txt          # Banco de dados de texto (gerado automaticamente)

# 🌡️ Monitor de Temperatura com ESP32 + DHT11

> Projeto desenvolvido como parte das atividades da **EletronJun - FCTE**.

---

## ⚙️ Configuração e Instalação

### 1. Bibliotecas Requeridas

Certifique-se de ter as seguintes bibliotecas instaladas na sua IDE Arduino:

| Biblioteca | Origem |
|---|---|
| DHT sensor library | Adafruit |
| WiFi | Nativo do ESP32 |
| WebServer | Nativo do ESP32 |
| SPIFFS | Nativo do ESP32 |

---

### 2. Configuração de Rede

Edite as seguintes linhas no arquivo `.ino` com as credenciais da sua rede local:

```cpp
const char *ssid = "NOME_DO_SEU_WIFI";
const char *password = "SENHA_DO_SEU_WIFI";
```

---

### 3. Upload dos Arquivos

1. Realize o **Upload do Código** para o ESP32.
2. Utilize a ferramenta **ESP32 Sketch Data Upload** para carregar o conteúdo da pasta `data` (HTML, CSS, JS) para o sistema de arquivos SPIFFS.

---

### 4. Uso

Após o boot, abra o **Monitor Serial** (115200 baud) para encontrar o endereço IP gerado. Digite este IP em qualquer navegador conectado à mesma rede.

---

## 📊 Endpoints da API (Server-side)

O ESP32 atua como um servidor que processa as seguintes rotas:

| Rota | Método | Descrição |
|---|---|---|
| `/` | GET | Carrega a interface `index.html`. |
| `/read` | GET | Retorna um JSON com dados do sensor, alertas e estados. |
| `/update?state=X` | GET | Altera o modo do sistema (0, 1 ou 2). |
| `/config` | GET | Altera unidade (°C/°F) e limite de temperatura (Modo 2 apenas). |
| `/download` | GET | Realiza o download do arquivo `log.txt` para o computador. |

---

## 🛡️ Segurança e Estabilidade

- **Sincronização NTP:** O relógio interno é sincronizado via `pool.ntp.org` (Fuso horário UTC-3), garantindo que os logs tenham data e hora corretas.

- **Watchdog de Sensor:** O sistema detecta se o sensor DHT11 foi desconectado, exibindo um alerta de erro na interface e interrompendo leituras falsas.

- **Bloqueio de Configuração:** Alterações de parâmetros (unidade e temperatura máxima) só são permitidas quando o sistema está no **Modo 2 (Ligado)**, prevenindo bugs no modo automático.

---

> **Nota:** Este projeto foi desenvolvido como parte das atividades da **EletronJun - FCTE**.
