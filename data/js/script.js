const notifications = [];
let lastAlertaTemp = false;
let lastAlertaHumi = false;
let lastSensorOk = null;
let currentMode = 0;

function adicionarNotificacao(mensagem) {
  const horario = new Date().toLocaleTimeString();

  notifications.unshift({ mensagem, horario });

  if (notifications.length > 20) {
    notifications.pop();
  }

  renderizarNotificacoes();
}

function renderizarNotificacoes() {
  const lista = document.getElementById("notifList");

  if (notifications.length === 0) {
    lista.innerHTML =
      '<div class="notif-empty">Nenhuma notificação ainda</div>';
    return;
  }

  lista.innerHTML = notifications
    .map(
      (n) => `
        <div class="notif-item">
          <div class="notif-msg">${n.mensagem}</div>
          <div class="notif-time">${n.horario}</div>
        </div>
      `,
    )
    .join("");
}

function setControlesLigado(habilitado) {
  document.getElementById("tempMaxInput").disabled = !habilitado;
  document.getElementById("unitSelect").disabled = !habilitado;
  document.getElementById("saveConfigBtn").disabled = !habilitado;
  document.getElementById("configLock").innerText = habilitado
    ? "Edição liberada"
    : "Disponível no modo Ligado";
}

function mudarModo(valor) {
  fetch("/update?state=" + valor)
    .then((res) => {
      if (!res.ok) throw new Error("Falha ao atualizar modo");
      console.log("Modo alterado para " + valor);
    })
    .catch((err) => console.log(err));

  if (valor == 0) {
    aplicarModoDesligado();
  }
}

function atualizarRelogio() {
  document.getElementById("time").innerText = new Date().toLocaleTimeString();
}

function atualizarStatusSensor(sensorOk) {
  const status = document.getElementById("sensorStatus");

  if (sensorOk) {
    status.innerText = "Sensor conectado";
    status.classList.remove("offline");
    status.classList.add("online");
  } else {
    status.innerText = "Sensor removido";
    status.classList.remove("online");
    status.classList.add("offline");
  }

  if (lastSensorOk === true && sensorOk === false) {
    adicionarNotificacao("Sensor removido ou leitura inválida");
  }

  lastSensorOk = sensorOk;
}

function aplicarDados(data) {
  const tElem = document.getElementById("t");
  const hElem = document.getElementById("h");
  const tempBox = document.getElementById("tempBox");
  const humiBox = document.getElementById("humiBox");
  const unit = data.unit || "C";

  currentMode = Number(data.modo ?? currentMode);
  setControlesLigado(currentMode === 2);

  tElem.innerText = data.temp;
  hElem.innerText = data.humi;

  const tempMaxInput = document.getElementById("tempMaxInput");
  const unitSelect = document.getElementById("unitSelect");

  document.getElementById("tempUnit").innerText = "°" + unit;

  if (document.activeElement !== unitSelect) {
    unitSelect.value = unit;
  }
  if (document.activeElement !== tempMaxInput) {
    tempMaxInput.value = data.tempMax;
  }

  atualizarStatusSensor(!!data.sensorOk);

  // Alerta de temperatura
  if (data.alertaTemp) {
    tempBox.classList.add("alerta-piscando");
    if (!lastAlertaTemp) {
      adicionarNotificacao(
        "Temperatura acima do limite (" + data.temp + " °" + unit + ")",
      );
    }
  } else {
    tempBox.classList.remove("alerta-piscando");
  }
  lastAlertaTemp = !!data.alertaTemp;

  // Alerta de umidade
  if (data.alertaHumi) {
    humiBox.classList.add("alerta-piscando");
    if (!lastAlertaHumi) {
      adicionarNotificacao("Umidade fora do intervalo (" + data.humi + " %)");
    }
  } else {
    humiBox.classList.remove("alerta-piscando");
  }
  lastAlertaHumi = !!data.alertaHumi;
}

function aplicarModoDesligado() {
  document.getElementById("t").innerText = "--";
  document.getElementById("h").innerText = "--";
  document.getElementById("tempBox").classList.remove("alerta-piscando");
  document.getElementById("humiBox").classList.remove("alerta-piscando");
  lastAlertaTemp = false;
  lastAlertaHumi = false;

  const status = document.getElementById("sensorStatus");
  status.innerText = "Sensor aguardando";
  status.className = "sensor-status waiting";
  lastSensorOk = null;

  setControlesLigado(false);
  currentMode = 0;
}

function atualizarDados() {
  atualizarRelogio();
  fetch("/read")
    .then((res) => res.json())
    .then((data) => {
      if (data.desligado) {
        aplicarModoDesligado();
      } else {
        aplicarDados(data);
      }
    })
    .catch(() => console.log("Aguardando dados..."));
}

function salvarConfiguracao() {
  const tempMax = document.getElementById("tempMaxInput").value;
  const unit = document.getElementById("unitSelect").value;
  const feedback = document.getElementById("configFeedback");

  feedback.innerText = "Salvando...";

  fetch("/config?tempMax=" + encodeURIComponent(tempMax) + "&unit=" + unit)
    .then((res) => {
      if (!res.ok)
        throw new Error("Configuração permitida apenas no modo Ligado");
      return res.json();
    })
    .then((data) => {
      aplicarDados(data);
      feedback.innerText = "Configuração salva para o modo Automático.";
      adicionarNotificacao(
        "Setpoint atualizado para " + data.tempMax + " °" + data.unit,
      );
    })
    .catch((err) => {
      feedback.innerText = err.message;
    });
}

// Inicialização
setControlesLigado(false);
atualizarDados();
setInterval(atualizarDados, 1000);
