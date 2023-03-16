#include <map>
#include <vector>
#include <unordered_map>

enum class command: std::uint16_t { 
// make it enum class
  C_ARITHMETIC
  , C_PUSH
  , C_POP
  , C_LABEL
  , C_GOTO
  , C_IF
  , C_FUNCTION
  , C_RETURN
  , C_CALL
};

std::map<std::string, enum command> commandsType = {
  {"add", command::C_ARITHMETIC}
  , {"sub", command::C_ARITHMETIC}
  , {"neg", command::C_ARITHMETIC}
  , {"eq", command::C_ARITHMETIC}
  , {"gt", command::C_ARITHMETIC}
  , {"lt", command::C_ARITHMETIC}
  , {"and", command::C_ARITHMETIC}
  , {"or", command::C_ARITHMETIC}
  , {"not", command::C_ARITHMETIC}
  , {"push", command::C_PUSH}
  , {"pop", command::C_POP}
  , {"label", command::C_LABEL}
  , {"if-goto", command::C_IF}
  , {"goto", command::C_GOTO}
  , {"function", command::C_FUNCTION}
  , {"return", command::C_RETURN}
  , {"call", command::C_CALL}
};

std::map<std::string, std::string> operators = {
  {"add", "+"}
  , {"sub", "-"}
  , {"neg", "-"}
  , {"and", "&"}
  , {"or", "|"}
  , {"not", "!"}
  , {"eq", "JNE"}
  , {"gt", "JGE"}
  , {"lt", "JLE"}
};

std::unordered_map<std::string, std::string> memAddr = {
  {"argument", "ARG"}
  , {"local", "LCL"}
  , {"this", "THIS"}
  , {"that", "THAT"}
};

std::vector<std::string> memSegs = {"THAT", "THIS", "ARG", "LCL"};
