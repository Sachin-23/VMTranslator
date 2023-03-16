#include <iostream>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vm_constants.h"
#include "parser.h"
#include "codeWriter.h"

void translate(Parser&, CodeWriter&);

int main(int argc, char **argv) {
  // Check for arguments passed
  if (argc < 2 || argc > 3) {
    std::cerr << "usage: VMTranslator input_filename.vm (optional)output_filename.asm" << std::endl;
    exit(1);
  }

  std::string path = argv[1], file;
  struct stat pathStat;
  stat(path.c_str(), &pathStat);

  if (S_ISDIR(pathStat.st_mode)) {
    std::cout << "Processing directory: " << path << std::endl;
    DIR *dir;

    dir = opendir(path.c_str()); 

    if (dir == NULL) {
      std::cerr << "Unable to open directory." << std::endl;
      exit(1);
    }

    uint64_t pos;
    if ((pos = path.find_last_of("/")) == path.length() - 1)
      path = path.substr(0, pos);

    file = path;
    if ((pos = path.find_last_of("/")) != std::string::npos)
      file = path.substr(path.find_last_of("/"));

    struct dirent *entry;

    std::cout << file << std::endl;

    // Initialize a code writer for directory
    CodeWriter codeWriter(path + "/" + file);
    // Add Bootstrap code
    codeWriter.writeInit();

    while ((entry=readdir(dir))) {
      file = entry -> d_name;
      // Initialize parser for every .vm file
      if (file.substr(file.find_last_of(".") + 1) == "vm") {
        Parser parser(path + "/" + file);
        codeWriter.setFileName(file);
        translate(parser, codeWriter);
      }
    }
    closedir(dir);
  }
  else if (S_ISREG(pathStat.st_mode)) {
    std::cout << "Processing file: " << path << std::endl;
    file = path;
    Parser parser(file);
    CodeWriter codeWriter(file);
    codeWriter.setFileName(file);
    translate(parser, codeWriter);
  }
  else {
    std::cerr << "May be a Symbolic Link or type is not recognized" << std::endl;
    exit(1);
  }
  /*
  try { 
   }
  catch (const std::runtime_error e) {
    std::cout << e.what() << std::endl; 
  }
  */
  return 0;
}

void translate(Parser &parser, CodeWriter &codeWriter) {
  while (parser.hasMoreLines()) {
    parser.advance();

    command cmdType = parser.commandType();

    switch (cmdType) {
      case command::C_ARITHMETIC: 
        codeWriter.writeArithmetic(parser.arg1());
        break;
      case command::C_PUSH: 
      case command::C_POP: 
        codeWriter.writePushPop(cmdType, parser.arg1(), parser.arg2());
        break;
      case command::C_LABEL: 
        codeWriter.writeLabel(parser.arg1());
        break;
      case command::C_GOTO: 
        codeWriter.writeGoto(parser.arg1());
        break;
      case command::C_IF:
        codeWriter.writeIf(parser.arg1());
        break;
      case command::C_FUNCTION:
        codeWriter.writeFunction(parser.arg1(), parser.arg2());
        break;
      case command::C_RETURN:
        codeWriter.writeReturn();
        break;
      case command::C_CALL:
        codeWriter.writeCall(parser.arg1(), parser.arg2());
        break;
    }
  }
}


