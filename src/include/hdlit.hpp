#ifndef HDLIT_HPP
#define HDLIT_HPP

#include <string>
#include <vector>
#include <fstream>

void findline(std::ifstream& file, std::vector<std::string>& lines, const std::string& keyword);

std::string gettop(const std::string& line);
std::vector<std::string> getinput(const std::string& line);
std::vector<std::string> getoutput(const std::string& line);
std::vector<std::string> getparameter(const std::string& line);

void print_instantiation_template(
    const std::string& top,
    const std::vector<std::string>& parameters,
    const std::vector<std::string>& ports
);

#endif