#include <unordered_map>

using namespace std;

class Frame {

 public:
  Frame (uint32_t returnAddress): returnAddress(returnAddress){}
  uint32_t getVariable(int key){
    return (variables.find(key) != variables.end()) ? variables[key] : 0; 
  }

  void setVariable(int key, int value){
    variables[key] = value; 
  }

  uint32_t getReturnAddress(){
    return returnAddress;
  }

 private:
  uint32_t returnAddress;
  unordered_map<int, int> variables;
};
