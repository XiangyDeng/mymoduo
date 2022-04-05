
#include <time.h>
#include <stdio.h>
#include <ctime>
#include <unistd.h>

using namespace std;

bool stampDetect(const float data[], int len);

int main() {
  float data[1000];
  int len = sizeof(data) / sizeof(data[0]);

  for (int i = 0; i < len; ++i) {
    data[i] = 1000.0;
  }

  data[15] = 0.0;
  data[16] = 0.0;
  data[17] = 0.0;
  
  data[503] = 0.0;
  data[504] = 0.0;
  data[505] = 0.0;

  if (stampDetect(data, len) == false) {
    printf("detect errno. \n");
  }

  return 0;
}

bool stampDetect(const float data[], int len) {

  enum Estate {
    firstUp,
    firstDown,
    secondUp
  };

  Estate state = firstUp;

  const float lowWaterMark = 10.0;
  const float highWaterMark = 900.0;

  time_t lowTimestamp;
  time_t highTimestamp;
  tm *tm_time;
  float diff = 0.0; 
  char buf[128] = {0};

  for (int i = 1; i < len - 1; ++i) {    

    switch (state)
    {
    case firstUp:
      if (data[i - 1] < lowWaterMark && data[i] < lowWaterMark && data[i + 1] < lowWaterMark) {
        state = firstDown;
        time(&lowTimestamp);
      }
      break;

    case firstDown:
      if (data[i - 1] > highWaterMark && data[i] > highWaterMark && data[i + 1] > highWaterMark) {
        state = secondUp;
        sleep(1);
      }

      time(&highTimestamp);
      diff = difftime(highTimestamp, lowTimestamp);
      if (diff > 1) {
          return false;
      }
      break;

    case secondUp: 
      tm_time = localtime(&lowTimestamp);
      snprintf(buf, 128, "[%04d/%02d/%02d %02d:%02d:%02d]",
                          tm_time->tm_year + 1900,
                          tm_time->tm_mon + 1,
                          tm_time->tm_mday,
                          tm_time->tm_hour,
                          tm_time->tm_min,
                          tm_time->tm_sec);

      printf("transfer time is : %s \n", buf);
      state = firstUp;
      break;

     }
    }

  return true;
}

  
