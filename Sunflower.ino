#include "solar.hpp"
#include "page.hpp"

// Inicialização de variáveis utilizando o tipo solar_t que contém a estrutura de dados
solar_t sol;

void setup()
{

  // Inicialização da comunicação serial
  Serial.begin(115200);

  // Inicialização de variávies necessárias para o cálculo
  sol._lat = -23;
  sol._lon = -46;
  sol._d = 200;
  sol._lt = 11;
  sol._gmt = 14;

  // Cálculo dos valores utilizando as funções
  sol._lstm = LSTM(sol._lt, sol._gmt);
  sol._b = B(sol._d);
  sol._eot = EoT(sol._b);
  sol._tc = TC(sol._lstm, sol._lon, sol._eot);
  sol._lst = LST(sol._lt, sol._tc);
  sol._hra = HRA(sol._lst);
  sol._delta = DELTA(sol._b);
  sol._alpha = ALPHA(sol._delta, sol._lat, sol._hra);

  /**Serial.print("Lat: ");
  Serial.println(sol._lat);

  Serial.print("Lon: ");
  Serial.println(sol._lon);

  Serial.print("Dias: ");
  Serial.println(sol._d);

  Serial.print("Hora local: ");
  Serial.println(sol._lt);

  Serial.print("Hora GMT: ");
  Serial.println(sol._gmt);

  Serial.print("LSTM: ");
  Serial.println(sol._lstm);

  Serial.print("B: ");
  Serial.println(sol._b);

  Serial.print("EoT: ");
  Serial.println(sol._eot);

  Serial.print("Tc: ");
  Serial.println(sol._tc);

  Serial.print("LST: ");
  Serial.println(sol._lst);

  Serial.print("HRA: ");
  Serial.println(sol._hra);

  Serial.print("Delta: ");
  Serial.println(sol._delta);

  Serial.print("Alpha: ");
  Serial.println(sol._alpha);*/
}

void loop()
{
}
