#include <unordered_map>

using namespace std;

class Frame {

 public:
  uint32_t getVariable(int key){
    return (variables.find(key) != variables.end()) ? variables[key] : 0; 
  }

  void setVariable(int key, int value){
    variables[key] = value; 
  }

 private:
  unordered_map<int, int> variables;
};
