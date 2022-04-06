#include <stdio.h>
#include <stdlib.h>

// 单位： rad / s
float slopeDetectTask(const float data[], int len) {
  const float INTERVAL = 1;

  float preDiff = data[1] - data[0];
  float preVal = data[0];
  float slope = 0.0;
  
  for (int i = 1; i < len; ++i) {
    float curVal = data[i];
    float diff = curVal - preVal;
    if (diff - preDiff > 50) {
      diff = preDiff + (diff - preDiff) * 0.1;
    } else if (diff - preDiff > 100) {
      diff = preDiff + (diff - preDiff) * 0.045;
    }

    slope += diff;
    preDiff = diff;
    preVal = curVal;
  }

  return slope / ((len - 1) * INTERVAL);
}

int main() {
  
  float data[100];
  int size = sizeof(data) / sizeof(float);

  for (int i = 0; i < 50; ++i) {
    data[i] = i*5;
  }

  for (int i = 50; i < 100; ++i) {
    data[i] = i*7;
  }

  printf( "slope is: %f \n", slopeDetectTask(data, size));

  return 0;
}


