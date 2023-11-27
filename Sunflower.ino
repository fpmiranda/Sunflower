#include <ESP32Servo.h>
#include <ESPAsyncWebSrv.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"
#include "solar.hpp"
#include "page.hpp"

// Servo guide
// https://docs.arduino.cc/learn/electronics/servo-motors

// Definição do intervalo para simulação
#define interval 2000

// Inicialização das constantes
const char *ssid = "";
const char *senha = "";
const char *PARAM_MESSAGE = "lat";

// Deeclaração do timer
unsigned long timer = 0;
float anguloAnterior = 0;

// Declaração da FSM
enum
{
  RUN,
  PAUSE,
  SIM,
  STOP
} stt;

// Inicialização do servidor web
AsyncWebServer server(80);

// Inicialização de variáveis utilizando o tipo solar_t que contém a estrutura de dados
solar_t solar;

// Inicialização do servo
Servo servo;

// Função para calcular o angulo que o servo do alpha deve se mover
float calculaAngulo(float L, float alpha)
{
  return (360 * L * cos(alpha)) / 8.14; // 8.14 é o perímetro da engrenagem
}

// Função para imprimir valores da estrutura solar_t
void imprimeValores(solar_t &sol)
{
  Serial.println();
  Serial.println(sol._lstm);
  Serial.println(sol._eot);
  Serial.println(sol._tc);
  Serial.println(sol._lst);
  Serial.println(sol._hra);
  Serial.println(sol._delta);
  Serial.println(sol._alpha);
  Serial.println(sol._azimuth);
  Serial.println();
}

// Função para cálculo dos valores
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
  sol._azimuth = AZIMUTH(sol._delta, sol._lat, sol._hra);
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
  else if (var == "AZIMUTH")
    return String(solar._azimuth);
}

// Função para ativar o servo motor no sentido horário
void ativaServoHorario(Servo &servo, int pin, int angulo)
{
  servo.attach(pin);
  for (int i = 0; i < angulo; i++)
  {
    servo.write(angulo);
    delay(15);
  }
  servo.detach();
}

// Função para ativar o servo motor no sentido anti-horário
void ativaServoAntiHorario(Servo &servo, int pin, int angulo)
{
  servo.attach(pin);
  for (int i = angulo; i >= 0; i--)
  {
    servo.write(angulo);
    delay(15);
  }
  servo.detach();
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
  solar._lat = 0;
  solar._lon = 0;
  solar._d = 0;
  solar._lt = 0;
  solar._gmt = 0;

  // Inicialização da FSM
  stt = RUN;

  // Inicialização do timer
  timer = millis();

  // Cálculo dos valores utilizando as funções
  calculaValores(solar);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });

  server.on("/calc", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if(request->hasParam(PARAM_MESSAGE)){
                solar._lat = request->getParam("lat")->value().toFloat();
                solar._lon = request->getParam("lon")->value().toFloat();
                solar._d = request->getParam("d")->value().toFloat();

                if(request->getParam("sim")->value().toInt() == 1) stt = SIM;
                else {
                  stt = RUN;
                  solar._lt = request->getParam("lt")->value().toFloat();
                  solar._gmt = request->getParam("gmt")->value().toFloat();
                }

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
              json["azimuth"] = String(solar._azimuth);

              // Envia os valores de lt e gmt apenas se a FSM estiver em modo de simulação
              if(stt == SIM){
                json["hora"] = String((int)solar._lt);
                json["gmt"] = String(solar._gmt);
              }

              serializeJson(json, *response);
              request->send(response); });

  server.begin();
}

void loop()
{
  switch (stt)
  {
  case RUN:
    // Não implementado pois só usaremos o modo simulação
    break;

  case PAUSE:
    // Não implementado
    break;

  case SIM:
    // servo.attach(15);
    //  A cada intervalo de tempo, incrementa os valores de lt e gmt e reseta timer
    if (millis() - timer > interval)
    {
      calculaValores(solar);
      imprimeValores(solar);
      Serial.println(anguloAnterior);

      // Calcula valores para controle dos motores com base no angulo atual e no anterior
      int alphaAngulo = calculaAngulo(9, solar._alpha);

      // Calcula o angulo que o servo deve se mover com base no comprimento da base e no angulo alpha e move o motor
      Serial.println(alphaAngulo); // Simula o motor
      // ativaServoAntiHorario(servo, 15, alphaAngulo - anguloAnterior);

      // Incrementa os valores de lt e gmt durante o período de simulação
      if (solar._lt > 6 && solar._lt < 18)
      {
        anguloAnterior = solar._alpha;
        solar._lt++;
        solar._gmt++;
      }
      else
        stt = STOP;

      timer = millis();
    }
    break;

  case STOP:
    // Não implementado
    break;

  default:
    // Não faz nada
    break;
  }
}
