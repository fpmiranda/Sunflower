#include <ESPAsyncWebSrv.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"
#include "solar.hpp"
#include "page.hpp"

const char *ssid = "";
const char *senha = "";

const char *PARAM_MESSAGE = "lat";

// Inicialização do servidor web
AsyncWebServer server(80);

// Inicialização de variáveis utilizando o tipo solar_t que contém a estrutura de dados
solar_t solar;

void calculaValores(solar_t &sol)
{
  // Cálculo dos valores utilizando as funções
  sol._lstm = LSTM(sol._lt, sol._gmt);
  sol._b = B(sol._d);
  sol._eot = EoT(sol._b);
  sol._tc = TC(sol._lstm, sol._lon, sol._eot);
  sol._lst = LST(sol._lt, sol._tc);
  sol._hra = HRA(sol._lst);
  sol._delta = DELTA(sol._b);
  sol._alpha = ALPHA(sol._delta, sol._lat, sol._hra);
}

// Função para lidar com erro 404
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

// Função que processa as variáveis da página web
String processor(const String &var)
{
  if (var == "LSTM")
    return String(solar._lstm);
  else if (var == "EoT")
    return String(solar._eot);
  else if (var == "TC")
    return String(solar._tc);
  else if (var == "LST")
    return String(solar._lst);
  else if (var == "HRA")
    return String(solar._hra);
  else if (var == "DELTA")
    return String(solar._delta);
  else if (var == "ALPHA")
    return String(solar._alpha);
}

void setup()
{

  // Inicialização da comunicação serial
  Serial.begin(115200);

  // Inicialização da conexão WiFi
  WiFi.begin(ssid, senha);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Conectando ao WiFi..");
  }

  // Imprime o IP Local
  Serial.println(WiFi.localIP());

  // Inicialização de variávies necessárias para o cálculo
  // solar._lat = -23;
  // solar._lon = -46;
  // solar._d = 200;
  // solar._lt = 11;
  // solar._gmt = 14;
  solar._lat = 0;
  solar._lon = 0;
  solar._d = 0;
  solar._lt = 0;
  solar._gmt = 0;

  // Cálculo dos valores utilizando as funções
  calculaValores(solar);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });

  server.on("/calc", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if(request->hasParam(PARAM_MESSAGE)){
                Serial.println("Recebendo parametros");
                solar._lat = request->getParam("lat")->value().toFloat();
                solar._lon = request->getParam("lon")->value().toFloat();
                solar._d = request->getParam("d")->value().toFloat();
                solar._lt = request->getParam("lt")->value().toFloat();
                solar._gmt = request->getParam("gmt")->value().toFloat();
                calculaValores(solar);
              }

              AsyncResponseStream *response = request->beginResponseStream("application/json");
              DynamicJsonDocument json(1024);
              json["lstm"] = String(solar._lstm);
              json["eot"] = String(solar._eot);
              json["tc"] = String(solar._tc);
              json["lst"] = String(solar._lst);
              json["hra"] = String(solar._hra);
              json["delta"] = String(solar._delta);
              json["alpha"] = String(solar._alpha);
              serializeJson(json, *response);
              request->send(response); });

  server.begin();
}

void loop()
{
}
