#include <vector>
#include <deque>
#include <iostream>
#include <string>
#include <stdexcept>
#include <stack>

#include "frame.h"
using namespace std;
using stackInstruction = uint32_t; 

enum class Instruction: stackInstruction {
  Push = 0x80000000,
  Mult = 0x80000001,
  Div = 0x80000002,
  Add = 0x80000003,
  Sub = 0x80000004,
  Halt = 0x80000005,
  And = 0x80000006,
  Or = 0x80000007,
  Not = 0x80000008,
  Pop = 0x80000009,
  Dup = 0x8000000A,
  Load = 0x8000000B,
  Store = 0x8000000C,
  Jmp = 0x8000000D,
  Jif = 0x8000000E,
  Iseq = 0x8000000F,
  Isgt = 0x80000010,
  Isge = 0x80000011,
  Call = 0x80000012,
  Ret = 0x80000013,
};

class CPU {
public:
  CPU(vector<stackInstruction> instructions): program(instructions) {
    Frame iFrame(0); 
    frames.push(iFrame); 
  }

  void step() {
    stackInstruction nextInstruction = getNextWordFromProgram();
    decodeInstruction(nextInstruction);
  }

  void run() {
    while (!halted) {
      step();
    }
  }

private:
  Frame getCurrentFrame(){
    return frames.top();
  }

  stackInstruction getNextWordFromProgram() {
    if (instruction_address >= program.size()) {
      throw runtime_error("stack underflow");
    }

    stackInstruction nextWord = program[instruction_address++];
    return nextWord;
  }

  bool isSpecialInstruction(stackInstruction element) {
    return (element & 0x80000000) != 0;
  }

  bool toBool(stackInstruction n){
    return n!=0;
  }

  stackInstruction toInt(bool b){
    return b ? 1 : 0;
  }

  void checkStackHasAtLeastOneItem(string instruction){
    if (stk.size() == 0)
      throw runtime_error("stack empty" + instruction);
  }

  void checkStackHasAtLeastTwoItems(){
    if (stk.size() <= 1){
      throw runtime_error("stack doesn't have 2 elements");
    }
  }

  void decodeInstruction(stackInstruction instruction) {
    if (!isSpecialInstruction(instruction)) {
      stk.push_front(instruction);
      return;
    }
    Instruction eIns = static_cast<Instruction>(instruction);

    switch(eIns) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Mult:
    case Instruction::Div:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Iseq:
    case Instruction::Isgt:
    case Instruction::Isge:{
      checkStackHasAtLeastTwoItems();
      stackInstruction n1 = stk.front(); stk.pop_front();
      stackInstruction n2 = stk.front(); stk.pop_front();
      stk.push_front(doBinaryOp(eIns, n1, n2));
      break;
    }

    case Instruction::Push: {
      stk.push_front(getNextWordFromProgram());
      break;
    }
    case Instruction::Call: {
      stackInstruction address = getNextWordFromProgram();
      // checkJumpAddress
      frames.push(Frame(instruction_address));
      instruction_address = address;
      break; 
    }

    case Instruction::Ret: {
      // checkThereIsAReturnAddress()
      uint32_t returnAddress = getCurrentFrame().getReturnAddress();
      frames.pop();
      instruction_address = returnAddress;
      break;
    }
    case Instruction::Halt: {
      halted = true;
      break;
    }
    case Instruction::Not: {
      stackInstruction num_1 = stk.front(); stk.pop_front();
      bool res = !toBool(num_1);
      stk.push_front(toInt(res));
      break;
    }
    case Instruction::Pop: {
      checkStackHasAtLeastOneItem("Pop");
      stk.pop_front();
      break;
    }
    case Instruction::Dup: {
      checkStackHasAtLeastOneItem("Dup");
      stackInstruction n = stk.front();
      stk.push_front(n);
      break;
    }

    case Instruction::Load:{
      stackInstruction varNumber = getNextWordFromProgram();
      stk.push_front(getCurrentFrame().getVariable(varNumber));
      break;
    }
    case Instruction::Store:{
      int varNumber = getNextWordFromProgram();
      checkStackHasAtLeastOneItem("Store");
      getCurrentFrame().setVariable(varNumber, stk.front());
      stk.pop_front();
      break;
    }
    case Instruction::Jmp:{
      stackInstruction new_instruction_address = getNextWordFromProgram();
      if (new_instruction_address >= program.size()){
        throw runtime_error("Invalid jump address");
      }
      instruction_address = new_instruction_address;
      break;
    }

    case Instruction::Jif:{
      stackInstruction new_instruction_address = getNextWordFromProgram();
      checkStackHasAtLeastOneItem("Jif");
      if (toBool(stk.front())){
        if (new_instruction_address < program.size())
          instruction_address = new_instruction_address;
      }
      break;
    }
    }
  }


  stackInstruction doBinaryOp(Instruction instruction, stackInstruction n1, stackInstruction n2){
    switch(instruction){
    case Instruction::Add:{
      return n1 + n2;
    }
    case Instruction::Sub:{
      return n1 - n2;
    }
    case Instruction::Mult:{
      return n1 * n2;
    }
    case Instruction::Div:{
      if(n2 == 0)
        throw runtime_error("div by zero.");
      return n1 / n2;
    }
    case Instruction::And:{
      bool res = toBool(n1) && toBool(n2);
      return toInt(res);
    }
    case Instruction::Or:{
      bool res = toBool(n1) || toBool(n2);
      return toInt(res);
    }
    case Instruction::Iseq:{
      bool res = (n1 == n2);
      return toInt(res);
    }
    case Instruction::Isgt:{
      bool res = (n1 > n2);
      return toInt(res);
    }
    case Instruction::Isge:{
      bool res = (n1 > n2) || (n1 == n2);
      return toInt(res);
    }
    default:
      throw runtime_error("wrong instruction");
    }
  }
  stack<Frame> frames; 
  vector<stackInstruction> program;
  deque<stackInstruction> stk;
  bool halted = false;
  size_t instruction_address = 0;
};


int main() {
  vector<stackInstruction> instructions = {
    static_cast<stackInstruction>(Instruction::Push), 10,
    static_cast<stackInstruction>(Instruction::Push), 20,
    static_cast<stackInstruction>(Instruction::Add),
    static_cast<stackInstruction>(Instruction::Halt)
  };

  CPU cpu(instructions);
  cpu.run();
  return 0;
}
