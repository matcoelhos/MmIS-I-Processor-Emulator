#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<sstream>

using namespace std;

/*---------------*/
/*Control Objects*/
/*---------------*/

//label control for memory access
class tags{
public:
  int addr;
  int size;
  string label;
};

vector<tags> labelAddrVec;

/*--------------------*/
/*Processor Components*/
/*--------------------*/

int PC = 0;
signed int DataMemory[4096];      //Memory of 256kB
vector<string> InstructionMem;    //"Infinity" instruction memory
signed int Sregisterers[16];      //Type S registerers
signed int Tregisterers[16];      //Type T registerers

/*------------------*/
/*Execution Funcions*/
/*------------------*/

void LW(int addr, int Value)
{
    if (addr < 16) Tregisterers[addr] = Value;
    else Sregisterers[addr%16] = Value;
}

void SW(int Value, int MemAddr)
{
  DataMemory[MemAddr] = Value;
}

void JUMP(string label)
{
  int search = 0;
  string searchstring = label + ":";
  while(InstructionMem[search].compare(searchstring) && search < InstructionMem.size())
  {
    search++;
  }
  if (search < InstructionMem.size()) PC = search;
}

void ADD(int addr, int V1, int V2)
{
  int ans = V1 + V2;
  LW(addr, ans);
}

void BNE(int V1, int V2, string label)
{
  if (V1 != V2) JUMP(label);
}

void SLT(int addr, int V1, int V2)
{
  if (V1 < V2) LW (addr, 1);
  else LW (addr, 0);
}

/*---------------*/
/*Pipeline Stages*/
/*---------------*/

vector<string> Fetch()
{
  vector<string> ans;
  string line = InstructionMem[PC];
  istringstream iss(line);
  do
  {
     string subs;
     iss >> subs;
     if (subs[subs.size()-1] == ',' || subs[subs.size()-1] == ':')
        subs.pop_back();
     if (subs.compare("\0")){
       ans.push_back(subs);
     }
  } while (iss);
  return ans;
}

void ALUExec(int op, int V1, int V2, int V3, string label)
{
  if (op == 1) ADD(V3, V1, V2);
  else if (op == 2) SLT(V3, V1, V2);
  else if (op == 3) BNE(V1, V2, label);
  else if (op == 4) JUMP(label);
}

int Mem(int op, int addr, int value)
{
  if (op == 0) return DataMemory[addr];
  else DataMemory[addr] = value;
}

void WriteReg(int value, int addr)
{
  LW(value,addr);
}

void ReadDecode(vector<string> args)
{
  /*ADD instruction*/
  if (!args[0].compare("add"))
  {
    int reg1, reg2=-1, regans;
    int V1, V2;
    char c;

    sscanf(args[2].c_str(), "$%c%d", &c, &reg1);
    if (c == 's') V1 = Sregisterers[reg1];
    else if (c == 't') V1 = Tregisterers[reg1];
    else if (c == 'z') V1 = 0;

    sscanf(args[3].c_str(), "$%c%d", &c, &reg2);
    if (reg2 == -1 && c != 'z') V2 = stoi(args[3]);
    else if (c == 's') V2 = Sregisterers[reg2];
    else if (c == 't') V2 = Tregisterers[reg2];
    else if (c == 'z') V2 = 0;

    sscanf(args[1].c_str(), "$%c%d", &c, &regans);
    if (c == 's') regans += 16;

    ALUExec(1, V1, V2, regans, "");
  }

  /*SLT instruction*/
  else if (!args[0].compare("slt"))
  {
    int reg1, reg2=-1, regans;
    int V1, V2;
    char c;

    sscanf(args[2].c_str(), "$%c%d", &c, &reg1);
    if (c == 's') V1 = Sregisterers[reg1];
    else if (c == 't') V1 = Tregisterers[reg1];
    else if (c == 'z') V1 = 0;

    sscanf(args[3].c_str(), "$%c%d", &c, &reg2);
    if (reg2 == -1 && c != 'z') V2 = stoi(args[3]);
    else if (c == 'z') V2 = 0;
    else
    {
      if (c == 's') V2 = Sregisterers[reg2];
      else if (c == 't') V2 = Tregisterers[reg2];
      else if (c == 'z') V2 = 0;
    }

    sscanf(args[1].c_str(), "$%c%d", &c, &regans);
    if (c == 's') regans += 16;

    ALUExec(2, V1, V2, regans, "");
  }

  /*LW instruction*/
  else if (!args[0].compare("lw"))
  {
    int reg1, reg2=-1;
    int V1, V2, shift = -1;
    char c;

    sscanf(args[1].c_str(), "$%c%d", &c, &reg1);
    if (c == 's') V1 = reg1+16;
    else if (c == 't') V1 = reg1;

    sscanf(args[2].c_str(), "%d($%c%d)", &shift, &c, &reg2);
    if (reg2 == -1)
    {
      int pos = 0;
      while (labelAddrVec[pos].label.compare(args[2])) pos++;
      if (pos < labelAddrVec.size()) V2 = labelAddrVec[pos].addr;
    }
    else
    {
      int addr;
      if (c == 's') V2 = Mem(0, Sregisterers[reg2]+shift, 0);
      else if (c == 't') V2 = Mem(0, Tregisterers[reg2]+shift, 0);
    }
    WriteReg(V1, V2);
  }

  /*SW instruction*/
  else if (!args[0].compare("sw"))
  {
    int reg1, reg2=-1;
    int V1, V2, shift = -1;
    char c;

    sscanf(args[1].c_str(), "$%c%d", &c, &reg1);
    if (c == 's') V1 = Sregisterers[reg1];
    else if (c == 't') V1 = Tregisterers[reg1];

    sscanf(args[2].c_str(), "%d($%c%d)", &shift, &c, &reg2);
    if (reg2 == -1)
    {
      int pos = 0;
      while (labelAddrVec[pos].label.compare(args[2])) pos++;
      if (pos < labelAddrVec.size()) V2 = labelAddrVec[pos].addr;
    }
    else
    {
      int addr;
      if (c == 's') V2 = Sregisterers[reg2]+shift;
      else if (c == 't') V2 = Tregisterers[reg2]+shift;
    }
    Mem(1, V2, V1);
  }

  /*BNE instruction*/
  else if (!args[0].compare("bne"))
  {
    int reg1, reg2=-1, regans;
    int V1, V2;
    char c;

    sscanf(args[1].c_str(), "$%c%d", &c, &reg1);
    if (c == 's') V1 = Sregisterers[reg1];
    else if (c == 't') V1 = Tregisterers[reg1];
    else if (c == 'z') V1 = 0;

    sscanf(args[2].c_str(), "$%c%d", &c, &reg2);
    if (reg2 == -1 && c != 'z') V2 = stoi(args[2]);
    else if (c == 'z') V2 = 0;
    else
    {
      if (c == 's') V2 = Sregisterers[reg2];
      else if (c == 't') V2 = Tregisterers[reg2];
      else if (c == 'z') V1 = 0;
    }

    ALUExec(3, V1, V2, 0, args[3]);
  }

  /*JUMP instruction*/
  else if (!args[0].compare("jump"))
  {
    ALUExec(4, 0, 0, 0, args[1]);
  }
}

/*----------------------*/
/*Emulation Main Funcion*/
/*----------------------*/

int main(int argc, char * argv[])
{
  ifstream f;
  string line;
  f.open(argv[1]);

  /*Searching for data section*/
  while (getline(f,line) && line.compare(".data"));
  int busy = 0;

  /*Storing data from data section*/
  while (getline(f,line) && line.compare(".program"))
  {
    if (line[0] != '#' && line.size() > 1)
    {
      istringstream iss(line);
      string subs;
      iss >> subs;
      tags t;
      t.label = subs;
      t.addr = busy;
      t.size = 0;
      while (iss)
      {
        iss >> subs;
        if (subs[subs.size()-1] == ',' || subs[subs.size()-1] == '=')
          subs.pop_back();
        if (subs.compare("\0")) {
          DataMemory[busy] = stoi(subs);
          t.size++;
          busy++;
        }
      }
      t.size--;
      busy--;
      DataMemory[busy] = 0;
      labelAddrVec.push_back(t);
    }
  }

  /*Storing instructions in instruction memory*/
  while (getline(f,line))
  {
    while(line[0] == ' ' || line[0] == '\t') line.erase(line.begin());
    if (line[0] != '#' && line.size() > 1)
    {
      InstructionMem.push_back(line);
    }
  }
  f.close();

  /*Execution Loop*/
  int clocks = 0;
  while(PC < InstructionMem.size())
  {
    vector<string> args;
    args = Fetch();
    ReadDecode(args);
    PC++;
    clocks++;
  }
  cout << "clocks: " << clocks << endl;
  cout << "Data: ";
  for (int i = 0; i < 20; i++) cout << DataMemory[i] << " ";
  cout << endl;
}
