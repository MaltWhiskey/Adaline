#include "Adaline.h"
#include <Arduino.h>
// Constructor initializes inputs and weights
Adaline::Adaline() {
  resetInputs();
  resetWeights();
}
// Default all inputs to -1
void Adaline::resetInputs() {
  for (uint8_t i = 0; i < n; i++)
    x[i] = -1;
}
// Default all weights to 0
void Adaline::resetWeights() {
  bias = 0;
  for (uint8_t i = 0; i < n; i++)
    w[i] = 0;
}
// Calculate the output of the Adaline
double Adaline::getOutput() {
  double y = 0;
  for (uint8_t i = 0; i < n; i++) {
    y += w[i] * x[i];
  }
  return y + bias;
}
// Defaults input i to -1
void Adaline::resetInput(uint8_t i) {
  if (i < n)
    x[i] = -1;
}
// Sets input i to +1
void Adaline::setInput(uint8_t i) {
  if (i < n)
    x[i] = +1;
}
// Inverts the value of input i
void Adaline::invertInput(uint8_t i) {
  if (i < n)
    x[i] *= -1;
}
// Get the value of input i
int8_t Adaline::getInput(uint8_t i) {
  if (i < n)
    return x[i];
  return 0;
}
// Get the value of weight i
double Adaline::getWeight(uint8_t i) {
  if (i < n)
    return w[i];
  return 0;
}
// Get the bias value
double Adaline::getBias() {
  return bias;
}
// Adjust weights and bias to make output more positive
void Adaline::makePositive() {
  iterate(+1 - getOutput());
}
// Adjust weights and bias to make output more negative
void Adaline::makeNegative() {
  iterate(-1 - getOutput());
}
//  Update weights and bias based on error
void Adaline::iterate(double error) {
  for (uint8_t i = 0; i < n; i++)
    w[i] += 2 * mu * error * x[i];
  bias += 2 * mu * error;
}