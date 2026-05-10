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
