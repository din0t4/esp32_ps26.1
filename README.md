# ESP32 Environmental Monitor - Technical Documentation

```json
{
  "project": {
    "name": "Sistema de Monitoramento EletronJun",
    "version": "1.0.0",
    "mcu": "ESP32-WROOM",
    "framework": "Arduino IDE / C++"
  },

  "hardware_layout": {
    "sensors": {
      "DHT11": { "pin": 15, "type": "Temperature/Humidity" }
    },
    "indicators": {
      "LED_1": { "pin": 12, "state": "Status/Mode 0" },
      "LED_2": { "pin": 26, "state": "Auto/Mode 1" },
      "LED_3": { "pin": 32, "state": "On/Mode 2" }
    }
  },

  "logic_modes": {
    "0": "DESLIGADO: Sistema em repouso. Apenas LED1 pulsa.",
    "1": "AUTOMATICO: Monitoramento ativo. Logs gravados a cada 120s no SPIFFS.",
    "2": "LIGADO: Configuração liberada. Ajuste de setpoints via Interface Web."
  },

  "safety_setpoints": {
    "LIMITE_TEMP": "30.0°C (Default - Alterável no modo 2)",
    "LIMITE_UMIDADE": "30.0% (Static)",
    "ALERTA_ACTION": "Todos os LEDs piscam em 100ms se ultrapassado"
  },

  "storage_system": {
    "file_system": "SPIFFS",
    "files": ["index.html", "css/styles.css", "js/script.js", "log.txt"],
    "time_sync": "NTP (pool.ntp.org) - GMT-3"
  },

  "api_rest_endpoints": {
    "GET /": "Serve index.html da memória flash",
    "GET /read": "Retorna JSON: {temp, humi, alertaTemp, alertaHumi, sensorOk, modo, unit}",
    "POST /update?state=X": "Altera modo de operação (X = 0, 1 ou 2)",
    "POST /config?unit=U&tempMax=T": "Altera unidade (C/F) e limite térmico",
    "GET /download": "Exporta histórico log.txt"
  },

  "deployment_steps": [
    "1. Configurar SSID/Password no código fonte",
    "2. Verificar instalação das bibliotecas DHT e ESP32 WiFi",
    "3. Realizar Upload do Sketch (.ino)",
    "4. Realizar Upload dos arquivos de dados (pasta /data) via SPIFFS Tool",
    "5. Acessar IP via Navegador para Dashboard"
  ],

  "maintainers": [
    { "name": "Equipe EletronJun", "campus": "FCTE" }
  ]
}
