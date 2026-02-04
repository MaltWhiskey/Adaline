#ifndef ADALINE_H_
#define ADALINE_H_
#include <stdint.h>
/*------------------------------------------------------------------------------
 * ADALINE CLASS
 *------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
class Adaline {
  public:
  static const uint8_t n = 16;

  private:
  double mu = 0.002;
  double bias = 0;
  double w[n] = {};
  int8_t x[n] = {};

  public:
  Adaline();
  void resetInputs();
  void resetWeights();
  double getOutput();

  void resetInput(uint8_t);
  void setInput(uint8_t);
  void invertInput(uint8_t);

  int8_t getInput(uint8_t);
  double getWeight(uint8_t);
  double getBias();
  void makePositive();
  void makeNegative();

  private:
  void iterate(double error);
};
#endif