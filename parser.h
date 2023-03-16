#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <sstream>
#include <stdio.h>

class Parser { 
  private: 
    std::ifstream inFile;
    std::string curCMD, token;
    command curCMDType;
    std::string curArg1;
    int curArg2 = -1;
    std::string line;

    std::string toRemove = " \n\r\t\f\v"; // all types of possible spaces to remove
    

    std::string leftTrim(std::string s){
      // finding the index just after white spaces
      uint64_t start = s.find_first_not_of(toRemove);
      // removed leading white spaces
      return (start == std::string::npos) ? "" : s.substr(start);
    }
    std::string rightTrim(std::string s){
      // finding the index just before white spaces
      uint64_t end = s.find_last_not_of(toRemove);
      // removed trailing white spaces
      return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }
    std::string trim(std::string s) {
      // Trim left and right white spaces
      return rightTrim(leftTrim(s));
    }

    std::string removeComment(std::string s) {
      uint64_t pos = s.find("//"); // finding the index of comment
      return (pos == std::string::npos) ? s : s.substr(0, pos); // removed comment
    }


  public: 
    Parser(std::string inputFile) {

      if (!(inputFile.substr(inputFile.find_last_of(".") + 1) == "vm")) {
        throw std::runtime_error(".vm extensions only");
      }

      // Stupid Bug
      //std::ifstream inFile(inputFile, std::ios::in);
      inFile.open(inputFile, std::ios::in);

      if (!inFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + inputFile);
    }


    ~Parser() {
      inFile.close();  
    }


    bool hasMoreLines() {
      // skip comments
      // get current position in file
      while (!inFile.eof()) {
        std::getline(inFile, line);
        line = trim(removeComment(line));
        if (line.length() > 1) {
          break;
        }
      }
      return !inFile.eof();
    }


    void advance() {
      // set the string stream at the beginning of the line  
      std::stringstream ss(line);
      ss >> token;
      curCMDType = commandsType[token];
      if (curCMDType != command::C_ARITHMETIC) {
        ss >> curArg1;
        // By Default to int conversion ?
        ss >> curArg2;
      }
      else {
        curArg1 = token; 
        // Just in case
        curArg2 = -1;
      }
    }
   

    command commandType() {
      return curCMDType; 
    }


    std::string arg1() {
      return curArg1;
    }


    int arg2() {
      return curArg2;       
    }
};

