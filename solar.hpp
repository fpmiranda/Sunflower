#include <math.h>

struct solar_t
{
  float _lt;
  float _gmt;
  float _d;
  float _lstm;
  float _b;
  float _eot;
  float _tc;
  float _lst;
  float _lon;
  float _lat;
  float _hra;
  float _delta;
  float _alpha;
  float _azimuth;
};

float degToRad(float deg)
{
  return (deg * 71) / 4068;
}

float radToDeg(float rad)
{
  return (rad * 4068) / 71;
}

float LSTM(int lt, int gmt)
{
  return 15 * (lt - gmt);
}

float B(float d)
{
  return 0.9863 * (d - 81);
}

float EoT(float b)
{
  return 9.87 * sin(2 * b) - 7.53 * cos(b) - 1.5 * sin(b);
}

float TC(float lstm, float lon, float eot)
{
  return 4 * (lon - lstm) + eot;
}

float LST(int lt, float tc)
{
  return lt + (tc / 60);
}

float HRA(float lst)
{
  return 15 * (lst - 12);
}

float DELTA(int b)
{
  return 23.45 * sin((b * 71) / 4068);
}

float ALPHA(float delta, float lat, float hra)
{
  return radToDeg(asin((sin(degToRad(delta)) * sin(degToRad(lat))) + (cos(degToRad(delta)) * cos(degToRad(lat)) * cos(degToRad(hra)))));
}

float AZIMUTH(float delta, float lat, float hra)
{
  return radToDeg(acos((sin(degToRad(delta)) * cos(degToRad(lat)) - cos(degToRad(delta)) * sin(degToRad(lat)) * cos(degToRad(hra))) / cos(degToRad(ALPHA(delta, lat, hra)))));
}