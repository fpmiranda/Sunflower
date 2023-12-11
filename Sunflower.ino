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
int alphaAnterior = 0;
Servo servo1;
Servo servo2;

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
void ativaServoHorario(Servo &servo, int pin, int angulo_a, int angulo_f)
{
  int i;
  servo.attach(pin);
  for (i = angulo_a; i <= angulo_f; i++)
  {
    servo.write(i);
    delay(15);
  }
}

// Função para ativar o servo motor no sentido anti-horário
void ativaServoAntiHorario(Servo &servo, int pin, int angulo_a, int angulo_f)
{
  int i;
  servo.attach(pin);
  for (i = angulo_a; i > angulo_f; i--)
  {
    servo.write(i);
    delay(15);
  }
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

                if(request->getParam("sim")->value().toInt() == 1 && stt == RUN){
                   stt = SIM;

                  // Coleta lt e gmt para achar o delta
                  solar._lt = request->getParam("lt")->value().toFloat();
                  solar._gmt = request->getParam("gmt")->value().toFloat();

                  // Calcula o delta gmt
                  int delta_gmt = solar._gmt - solar._lt;

                  // Altera o horário para o começo do dia
                  solar._lt = 6;
                  solar._gmt = (int)solar._lt + delta_gmt;
                }
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
                if(solar._lt < 10) json["hora"] = "0" + String((int)solar._lt);
                else json["hora"] = String((int)solar._lt);

                if(solar._gmt < 10) json["gmt"] = "0" + String((int)solar._gmt);
                else json["gmt"] = String((int)solar._gmt);
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
    servo1.detach();
    servo2.detach();
    // Não implementado pois só usaremos o modo simulação
    break;

  case PAUSE:
    // Não implementado
    break;

  case SIM:
    //  A cada intervalo de tempo, incrementa os valores de lt e gmt e reseta timer
    if (millis() - timer > interval)
    {
      calculaValores(solar);

      servo2.attach(2);

      if (solar._lt >= 6 && solar._lt <= 18)
      {

        // Ativa o servo 2 para acompanhar as horas
        servo2.write((solar._lt * 8) - 30);

        // Incrementa os valores de lt e gmt durante o período de simulação
        solar._lt++;
        solar._gmt++;

        // Ativa sentido horário ou antihorario dependendo do valor de alpha
        if (alphaAnterior < solar._alpha)
        {
          ativaServoHorario(servo1, 15, alphaAnterior, solar._alpha);
        }
        if (alphaAnterior >= solar._alpha)
        {
          ativaServoAntiHorario(servo1, 15, alphaAnterior, solar._alpha);
        }

        alphaAnterior = (int)solar._alpha;
      }
      else
      {
        stt = STOP;
        alphaAnterior = 0;
        servo1.detach();
        servo2.detach();
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
}