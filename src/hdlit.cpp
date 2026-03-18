#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <vector>
#include <cstdlib>
#include "include/hdlit.hpp"

/* Remove leading/trailing whitespace */
static std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) {
        start++;
    }

    std::size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) {
        end--;
    }

    return s.substr(start, end - start);
}

/* Remove trailing punctuation like ',', ';', ')' */
static std::string strip_trailing_punct(const std::string& s) {
    std::string out = trim(s);

    while (!out.empty() &&
          (out.back() == ',' || out.back() == ';' || out.back() == ')')) {
        out.pop_back();
    }

    return trim(out);
}

/* Basic identifier test */
static bool is_identifier_char(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

/* Split a declaration list by commas */
static std::vector<std::string> split_by_comma(const std::string& s) {
    std::vector<std::string> out;
    std::string current;

    for (char c : s) {
        if (c == ',') {
            if (!trim(current).empty()) {
                out.push_back(trim(current));
            }
            current.clear();
        } else {
            current += c;
        }
    }

    if (!trim(current).empty()) {
        out.push_back(trim(current));
    }

    return out;
}

/* Get last identifier from a chunk */
static std::string get_last_identifier(const std::string& text) {
    std::string token;
    std::string last;

    for (char c : text) {
        if (is_identifier_char(c)) {
            token += c;
        } else {
            if (!token.empty()) {
                last = token;
                token.clear();
            }
        }
    }

    if (!token.empty()) {
        last = token;
    }

    return strip_trailing_punct(last);
}

/* Find all lines containing keyword */
void findline(std::ifstream& file, std::vector<std::string>& lines, const std::string& keyword) {
    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(keyword) != std::string::npos) {
            lines.push_back(line);
        }
    }
}

/* module uart_rx #( ... */
std::string gettop(const std::string& line) {
    std::size_t pos = line.find("module");
    if (pos == std::string::npos) {
        return "";
    }

    pos += 6;

    while (pos < line.size() && std::isspace((unsigned char)line[pos])) {
        pos++;
    }

    std::string top;
    while (pos < line.size() && is_identifier_char(line[pos])) {
        top += line[pos];
        pos++;
    }

    return top;
}

/* input logic clk, rst; -> {"clk", "rst"} */
std::vector<std::string> getinput(const std::string& line) {
    std::vector<std::string> names;

    std::size_t pos = line.find("input");
    if (pos == std::string::npos) {
        return names;
    }

    std::string rest = line.substr(pos + 5);
    std::vector<std::string> chunks = split_by_comma(rest);

    for (const auto& chunk : chunks) {
        std::string name = get_last_identifier(chunk);
        if (!name.empty()) {
            names.push_back(name);
        }
    }

    return names;
}

/* output logic [7:0] a, b; -> {"a", "b"} */
std::vector<std::string> getoutput(const std::string& line) {
    std::vector<std::string> names;

    std::size_t pos = line.find("output");
    if (pos == std::string::npos) {
        return names;
    }

    std::string rest = line.substr(pos + 6);
    std::vector<std::string> chunks = split_by_comma(rest);

    for (const auto& chunk : chunks) {
        std::string name = get_last_identifier(chunk);
        if (!name.empty()) {
            names.push_back(name);
        }
    }

    return names;
}

/* parameter WIDTH = 8, DEPTH = 16 -> {"WIDTH", "DEPTH"} */
std::vector<std::string> getparameter(const std::string& line) {
    std::vector<std::string> names;

    std::size_t pos = line.find("parameter");
    if (pos == std::string::npos) {
        return names;
    }

    std::string rest = line.substr(pos + 9);
    std::vector<std::string> chunks = split_by_comma(rest);

    for (const auto& chunk : chunks) {
        std::size_t eq_pos = chunk.find('=');
        std::string left = (eq_pos != std::string::npos) ? chunk.substr(0, eq_pos) : chunk;

        std::string name = get_last_identifier(left);
        if (!name.empty()) {
            names.push_back(name);
        }
    }

    return names;
}

/* Print the generated instantiation template */
void print_instantiation_template(
    const std::string& top,
    const std::vector<std::string>& parameters,
    const std::vector<std::string>& ports
) {
    if (top.empty()) {
        std::cerr << "Error: no module name found.\n";
        return;
    }

    std::cout << top;

    if (!parameters.empty()) {
        std::cout << " #(\n";
        for (std::size_t i = 0; i < parameters.size(); i++) {
            std::cout << "    ." << parameters[i] << "(" << parameters[i] << ")";
            if (i + 1 < parameters.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << ")";
    }

    std::cout << " u_" << top << " (\n";

    for (std::size_t i = 0; i < ports.size(); i++) {
        std::cout << "    ." << ports[i] << "(" << ports[i] << ")";
        if (i + 1 < ports.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }

    std::cout << ");\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.sv|file.v>\n";
        return EXIT_FAILURE;
    }

    std::string filename = std::string(argv[1]);

    if (!filename.ends_with(".sv") && !filename.ends_with(".v")) {
        std::cerr << "Please input a Verilog or SystemVerilog file\n";
        return EXIT_FAILURE;
    }

    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "File " << argv[1] << " failed to open\n";
        return EXIT_FAILURE;
    }

    if (file.peek() == EOF) {
        std::cerr << "Input file: " << argv[1] << " is an empty file.\n";
        file.close();
        return EXIT_FAILURE;
    }

    std::vector<std::string> input_lines;
    std::vector<std::string> output_lines;
    std::vector<std::string> parameter_lines;
    std::vector<std::string> module_lines;

    findline(file, input_lines, "input");
    findline(file, output_lines, "output");
    findline(file, parameter_lines, "parameter");
    findline(file, module_lines, "module");

    std::string top;
    std::vector<std::string> parameters;
    std::vector<std::string> ports;

    for (const auto& line : module_lines) {
        std::string candidate = gettop(line);
        if (!candidate.empty()) {
            top = candidate;
            break;
        }
    }

    for (const auto& line : parameter_lines) {
        std::vector<std::string> found = getparameter(line);
        parameters.insert(parameters.end(), found.begin(), found.end());
    }

    for (const auto& line : input_lines) {
        std::vector<std::string> found = getinput(line);
        ports.insert(ports.end(), found.begin(), found.end());
    }

    for (const auto& line : output_lines) {
        std::vector<std::string> found = getoutput(line);
        ports.insert(ports.end(), found.begin(), found.end());
    }

    print_instantiation_template(top, parameters, ports);

    file.close();
    return EXIT_SUCCESS;
}