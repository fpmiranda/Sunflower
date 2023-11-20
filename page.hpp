/* Página main.html será descrita como strings neste arquivo para compor a página web que será
servida pelo ESP32 */

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <title>Sunflower - Engenharia Unificada II</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style>
      #data_input {
        border: 1px solid #ccc;
        padding: 10px 10px 10px 10px;
        width: 750px;
        margin-top: 20px;
      }

      #info_output {
        border: 1px solid #ccc;
        width: 750px;
        padding: 10px 10px 10px 10px;
      }

      body {
        background-color: #f2f2f2;
        margin-left: 10px;
        margin-right: 10px;
      }

      .form {
        margin: 0px;
        padding: 5px 10px;
        border: 1px solid #ccc;
        border-radius: 4px;
      }

      div {
        margin-bottom: 10px;
        margin-left: 10px;
        margin-right: 10px;
      }

      label {
        padding-left: 10px;
        padding-right: 0px;
      }

      div #dados {
        margin-bottom: 0px;
        margin-left: 10px;
        margin-right: 10px;
      }

      h3 {
        color: #333;
        margin-left: 10px;
      }

      h2 {
        margin-left: 10px;
      }

      #container {
        display: flex;
        flex-wrap: wrap;
      }

      #canvas {
        background-color: lightblue;
        margin: 65px 10px 10px 100px;
      }
    </style>
  </head>
  <body>
    <div id="container">
      <div id="info">
        <h2>Sunflower - Engenharia Unificada II</h2>
        <div id="data_input">
          <p>
            <label>Cidade: </label>
            <select id="select">
              <option value="0">Selecione uma cidade...</option>
              <option value="1">Recife</option>
              <option value="2">Santo André</option>
              <option value="3">Bruxelas</option>
              <option value="4">Las Vegas</option>
            </select>
            <label>Editar: </label>
            <input type="checkbox" id="editar" />
            <input
              type="button"
              id="simular"
              value="Simular Dia"
              onclick="toggleSim()"
            />
          </p>
          <p>
            <label>Latitude:</label>
            <input type="number" id="latitude" class="form" disabled />
            <label>Longitude:</label>
            <input type="number" id="longitude" class="form" disabled />
          </p>
          <p>
            <label>Hora local: </label>
            <input type="time" id="hora" class="form" disabled />
            <label>Hora GMT: </label>
            <input type="time" id="gmt" class="form" disabled />
            <label>Data: </label>
            <input type="date" id="data" class="form" disabled />
          </p>
        </div>
        <h3>Informações</h3>
        <div id="info_output">
          <div>
            <label>Meridiano de hora local padrão (LSTM): </label>
            <label id="lstm">%LSTM%</label>
          </div>
          <div>
            <label>Equação do tempo (EoT): </label>
            <label id="eot">%EoT%</label>
          </div>
          <div>
            <label>Fator de correção solar (TC): </label>
            <label id="tc">%TC%</label>
          </div>
          <div>
            <label>Horário solar local (LST): </label>
            <label id="lst">%LST%</label>
          </div>
          <div>
            <label>Ângulo horário (HRA): </label>
            <label id="hra">%HRA%</label>
          </div>
          <div>
            <label>Declinação solar (δ): </label>
            <label id="delta">%DELTA%</label>
          </div>
          <div>
            <label>Ângulo de elevação solar (α): </label>
            <label id="alpha">%ALPHA%</label>
          </div>
          <div>
            <label>Azimute: </label>
            <label id="azimuth">%AZIMUTH%</label>
          </div>
        </div>
      </div>
    </div>
  </body>
  <script>
    // Seleciona elementos do formulario
    var edit = document.getElementById("editar");
    var form = document.getElementsByClassName("form");
    var select = document.getElementById("select");
    var horario = document.getElementById("hora");
    var gmt = document.getElementById("gmt");
    var data = document.getElementById("data");
    var latitude = document.getElementById("latitude");
    var longitude = document.getElementById("longitude");
    var simular = document.getElementById("simular");

    // Seleciona elementos da tabela
    var lstm = document.getElementById("lstm");
    var eot = document.getElementById("eot");
    var tc = document.getElementById("tc");
    var lst = document.getElementById("lst");
    var hra = document.getElementById("hra");
    var delta = document.getElementById("delta");
    var alpha = document.getElementById("alpha");

    // Separa dia, mes ano, hora e minuto gmt e local
    var date = new Date();
    var dia = date.getDate();
    var mes = date.getMonth() + 1;
    var ano = date.getFullYear();
    var hora = date.getHours();
    var minuto = date.getMinutes();
    var utchora = date.getUTCHours();
    var utcminuto = date.getUTCMinutes();

    // Formata a data e hora
    if (dia < 10) dia = "0" + dia;
    if (mes < 10) mes = "0" + mes;
    if (hora < 10) hora = "0" + hora;
    if (minuto < 10) minuto = "0" + minuto;
    if (utchora < 10) utchora = "0" + utchora;
    if (utcminuto < 10) utcminuto = "0" + utcminuto;

    // Calcula quantos dias se passaram
    var inicio = new Date(ano, 0, 1);
    var agora = new Date(ano, mes - 1, dia);
    var numeroDeDias = Math.ceil((agora - inicio) / 8.64e7);

    var sim = false;

    // Função que ativa simulação do dia
    function toggleSim() {
      sim = !sim;
      simular.value = sim ? "Parar" : "Simular Dia";
    }

    // Atualiza os valores a cada segundo
    setInterval(() => {
      if (edit.checked == false && sim == false) {
        data.value = ano + "-" + mes + "-" + dia;
        horario.value = hora + ":" + minuto;
        gmt.value = utchora + ":" + utcminuto;
      }
    }, 500);

    setInterval(() => {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = () => {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var json = JSON.parse(xhr.responseText);
          lstm.innerHTML = json.lstm;
          eot.innerHTML = json.eot;
          tc.innerHTML = json.tc;
          lst.innerHTML = json.lst;
          hra.innerHTML = json.hra;
          delta.innerHTML = json.delta;
          alpha.innerHTML = json.alpha;
          azimuth.innerHTML = json.azimuth;

          if (sim == true) {
            horario.value = json.hora;
            gmt.value = json.gmt;
          }
        }
      };
      xhr.open(
        "GET",
        "/calc?" +
          "lt=" +
          hora +
          "&d=" +
          numeroDeDias +
          "&gmt=" +
          utchora +
          "&lat=" +
          latitude.value +
          "&lon=" +
          longitude.value +
          "&sim=" +
          sim,
        true
      );
      xhr.send();
    }, 1000);

    // toggle o formulario e limpa os campos
    edit.addEventListener("click", function () {
      for (var i = 0; i < form.length; i++) {
        if (form[i].disabled) {
          form[i].disabled = false;
          data.value = "";
          horario.value = "";
          gmt.value = "";
        } else {
          form[i].disabled = true;
        }
      }
    });

    // altera os valores de latitude e longitude conforme seleção
    select.addEventListener("change", function () {
      var option = select.options[select.selectedIndex].value;
      if (option == 0) {
        document.getElementById("latitude").value = "";
        document.getElementById("longitude").value = "";
      } else if (option == 1) {
        document.getElementById("latitude").value = -8.05;
        document.getElementById("longitude").value = -34.9;
      } else if (option == 2) {
        document.getElementById("latitude").value = -23.65;
        document.getElementById("longitude").value = -46.53;
      } else if (option == 3) {
        document.getElementById("latitude").value = 50.85;
        document.getElementById("longitude").value = 4.35;
      } else if (option == 4) {
        document.getElementById("latitude").value = 36.17;
        document.getElementById("longitude").value = -115.14;
      }
    });
  </script>
</html>
)rawliteral";