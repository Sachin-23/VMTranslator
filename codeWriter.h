#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>


class CodeWriter {
  private:
    std::ofstream outFile;
    std::string fileName;
    std::string funcName;;
    std::uint64_t jmpCounter = 0;
    std::uint64_t callCounter = 0;
    std::uint64_t varCounter = 0;

  public:
    CodeWriter(std::string outputFile) {
      std::string outFilename = outputFile;
      uint64_t pos;

      if ((pos=outputFile.find_last_of(".")) != std::string::npos)
        outFilename = outFilename.substr(0, pos);

      outFile.open(outFilename + ".asm");

      if (!outFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + outputFile);
    }
    ~CodeWriter() {
       outFile.close(); 
    }

    void setFileName(std::string outputFile) {
      // staticFilename to `Filename.`
      uint64_t pos;
      fileName = outputFile;
      if ((pos=fileName.find_last_of("/")) != std::string::npos) {
        fileName = fileName.substr(pos + 1);
      }
      if ((pos=fileName.find(".vm")) != std::string::npos) {
        fileName = fileName.substr(0, pos);
      }
      else {
        std::cout << "Only .vm extension" << std::endl;
      }

      fileName = fileName + ".";
    }

    void setFunctionName(std::string name) {
      funcName = name;
    }

    void writeInit() {
      outFile << "// Bootstrap" << std::endl;
      outFile << "  @256\n"
        "  D=A\n"
        "  @SP\n"
        "  M=D\n";
      writeCall("Sys.init", 0);
    }
  
    void writeArithmetic(std::string cmd) {
      //outFile << "// " << cmd << std::endl;
      outFile << "// "   << cmd << std::endl;
      if (cmd == "neg" || cmd == "not") {
        outFile << "  @SP\n"
          "  A=M-1\n"
          "  M=" + operators[cmd] + "M\n";
      }
      else if (cmd == "eq"
              || cmd == "gt"
              || cmd == "lt") {
         outFile << "  @SP\n"
           "  M=M-1\n" 
           "  A=M\n"
           "  D=M\n"
           "  A=A-1\n"
           "  M=D-M\n"
           "  D=M\n";
         outFile << "  @ELSE_" << jmpCounter << "\n";
         outFile << "  D;" + operators[cmd] + "\n";
         outFile << "(IF_" << jmpCounter << ")\n";
         outFile << "  @SP\n"
           "  A=M-1\n"
           "  M=-1\n";
         outFile << "  @END_IF_" << jmpCounter << "\n";
         outFile << "  0;JMP\n";
         outFile << "(ELSE_" << jmpCounter << ")\n";
         outFile << "  @SP\n"
           "  A=M-1\n"
           "  M=0\n";
         outFile << "(END_IF_" << jmpCounter << ")\n";
         ++jmpCounter;
      }
      else {
        outFile << "  @SP\n"
          "  M=M-1\n"
          "  A=M\n"
          "  D=M\n"
          "  A=A-1\n"
          "  M=M" << operators[cmd] << "D\n";
      }
    }
    
    void writePushPop(command curCMDType, std::string arg1, uint64_t arg2) {
      if (curCMDType == command::C_PUSH) {
        outFile << "// "   << "push " << arg1 << ' ' << arg2 << std::endl;
        outFile << "  @";
        if (arg1 == "static") {
          outFile << fileName << arg2 << std::endl;
          outFile << "  D=M\n";
        }
        else if (arg1 == "pointer") {
          if (arg2 == 0)
            outFile << "THIS\n";
          else if (arg2 == 1)
            outFile << "THAT\n";
          outFile << "  D=M\n";
        }
        else if (arg1 == "constant") {
           outFile << arg2 << std::endl;
           outFile << "  D=A\n";
        }
        else if (arg1 == "temp") {
          outFile << 5 + arg2 << std::endl;
          outFile << "  D=M\n";
        }
        else {
         outFile << memAddr[arg1] << std::endl;
         outFile << "  D=M\n";
         outFile << "  @" << arg2 << std::endl;
         outFile << "  A=D+A\n"
           "  D=M\n";
        }
        outFile << "  @SP\n"
          "  M=M+1\n"
          "  A=M-1\n"
          "  M=D\n";
      }
      else if (curCMDType == command::C_POP) {
        outFile << "// "   << "pop " << arg1 << ' ' << arg2 << std::endl;
        outFile << "  @";
        if (arg1 == "static") {
          outFile << fileName << arg2 << std::endl;
          outFile << "  D=A\n";
        }
        else if (arg1 == "pointer") {
          if (arg2 == 0)
            outFile << "THIS\n";
          else if (arg2 == 1)
            outFile << "THAT\n";
          outFile << "  D=A\n";
        }
        else if (arg1 == "temp") {
          outFile << 5 + arg2 << std::endl;
          outFile << "  D=A\n";
        }
        else {
          outFile << memAddr[arg1] << std::endl;
          outFile << "  D=M\n";
          outFile <<   "  @" << arg2 << std::endl;
          outFile <<   "  D=D+A\n";
        }
        outFile << "  @R13\n" 
          "  M=D\n"
          "  @SP\n"
          "  M=M-1\n"
          "  A=M\n"
          "  D=M\n"
          "  @R13\n"
          "  A=M\n"
          "  M=D\n";
      }
    }  

  void writeLabel(std::string label) {
    outFile << "// label " <<  label << std::endl;
    outFile << "(";
    if (funcName.length() > 0) 
     outFile << funcName << "$";
    outFile << label << ")" << std::endl;
  }


  void writeGoto(std::string label) {
    outFile << "// goto " <<  label << std::endl;
    outFile << "  @";
    if (funcName.length() > 0) 
     outFile << funcName << "$";
    outFile << label << std::endl;
    outFile << "  0;JMP\n";
  }

  void writeIf(std::string label) {
    outFile << "// if-goto " <<  label << std::endl;
    outFile << "  @SP\n"
      "  M=M-1\n"
      "  A=M\n"
      "  D=M\n";
    outFile << "  @";
    if (funcName.length() > 0) 
     outFile << funcName << "$";
    outFile << label << std::endl;  
    outFile << "  D;JNE\n";
  }


  void writeFunction(std::string functionName, uint64_t numVars) {
    setFunctionName(functionName);
    outFile << "// function " <<  functionName << " " << numVars << std::endl;
    outFile << "(" << functionName << ")" << std::endl;
    outFile << "  @" << numVars << std::endl;
    outFile << "  D=A\n"
      "  @SP\n"
      "  M=M+D\n";
    if (numVars > 0) {
      outFile << "(" << functionName << "$InitLclVars." \
        << varCounter << ")" << std::endl; // above 80
      outFile <<  "  @LCL\n"
        "  D=D-1\n"
        "  A=M+D\n"
        "  M=0\n";
      outFile << "  @" << functionName << "$InitLclVars." \
        << varCounter << std::endl; // above 80
      outFile << "  D;JGT\n"; // check this
      ++varCounter;
    }
  }


  void writeCall(std::string functionName, uint64_t numArgs) {
    outFile << "// call " <<  functionName << " " << numArgs << std::endl;

    outFile << "  @" << functionName + "$ret." + \
      std::to_string(callCounter) << std::endl;
    outFile << "  D=A\n"
        "  @SP\n"
        "  M=M+1\n"
        "  A=M-1\n"
        "  M=D\n";

    for (int i = 3; i >= 0; --i) {
      outFile << "  @" << memSegs[i] << std::endl;
      outFile << "  D=M\n"
          "  @SP\n"
          "  M=M+1\n"
          "  A=M-1\n"
          "  M=D\n";
    }
    outFile << "  @SP\n"
      "  D=M\n"
      "  @5\n"
      "  D=D-A\n";
    outFile << "  @" << numArgs << std::endl;
    outFile << "  D=D-A\n"
      "  @ARG\n"
      "  M=D\n"
      "  @SP\n"
      "  D=M\n"
      "  @LCL\n"
      "  M=D\n";
   outFile << "  @" << functionName << std::endl;
   outFile <<   "  0;JMP\n";
   outFile << "(" << functionName << "$ret." << callCounter << ")" << std::endl;
   // inc call counter
   ++callCounter;
  }


  void writeReturn() {
    outFile << "// return " << std::endl;
    outFile << "// frame = LCL" << std::endl;
    outFile << "  @LCL\n"
      "  D=M\n"
      "  @frame\n"
      "  M=D\n"
      "// retAddr = *(frame-5)\n"
      "  @5\n"
      "  A=D-A\n"
      "  D=M\n"
      "  @retAddr\n"
      "  M=D\n"
      "// *ARG = pop()\n"
      "  @SP\n"
      "  M=M-1\n"
      "  A=M\n"
      "  D=M\n"
      "  @ARG\n"
      "  A=M\n"
      "  M=D\n"
      "// SP=ARG+1\n"
      "  @ARG\n"
      "  D=M+1\n"
      "  @SP\n"
      "  M=D\n";
    for (int i = 0; i < 4; ++i) {
      outFile << "  @" << i+1 << std::endl;
      outFile << "  D=A\n"
        "  @frame\n"
        "  D=M-D\n"
        "  A=D\n"
        "  D=M\n";
      outFile << "  @" << memSegs[i] << std::endl;
      outFile << "  M=D\n";
    }
    outFile << "  @retAddr\n"
      "  A=M\n"
      "  0;JMP\n";
  }
};
