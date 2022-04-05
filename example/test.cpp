#include <iostream> 
#include <vector>
#include <algorithm>

using namespace std;

int main() {
  vector<int> data = {1,2,3,4,2,2,3,4,5,6};

  for (auto it = data.begin(); it != data.end(); ++it) {
    if (*it == 2) {
      data.erase(it);
    }
  }

  for (auto &e : data) {
    cout << e << " ";
  }
  cout << endl;

  data = {1,2,3,4,2,2,3,4,5,6};
  auto eit = remove(data.begin(), data.end(), 2); 

  for (auto it = data.begin(); it != eit; ++it) {
    cout << *it << " ";
  }
}