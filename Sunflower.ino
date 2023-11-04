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
}

void loop()
{
}
