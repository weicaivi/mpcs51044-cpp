#include <iostream>
#include <streambuf>
#include <string>

class IndentStreamBuf;
class IndentStream;

std::ostream& indent(std::ostream& os);
std::ostream& unindent(std::ostream& os);

// streambuf that handles indentation
class IndentStreamBuf : public std::streambuf {
public:
    // Constructor takes an output stream to write to
    IndentStreamBuf(std::ostream& output) 
        : output_stream(output), 
          indent_level(0), 
          at_line_start(true),
          indent_width(4) {}
    
    // Set/get the indent level
    void increase_indent() { indent_level++; }
    void decrease_indent() { if (indent_level > 0) indent_level--; }
    int get_indent_level() const { return indent_level; }

protected:
    // Override the overflow method to handle each character
    virtual int_type overflow(int_type c) override {
        if (c != traits_type::eof()) {
            // Handle newline - set at_line_start flag
            if (c == '\n') {
                at_line_start = true;
                output_stream.put(c);
                return c;
            }
            
            // If we're at the start of a line, add appropriate indentation
            if (at_line_start) {
                at_line_start = false;
                std::string indent_str(indent_level * indent_width, ' ');
                output_stream << indent_str;
            }
            
            // Output the current character
            output_stream.put(c);
        }
        return c;
    }

private:
    std::ostream& output_stream;
    int indent_level;
    bool at_line_start;
    const int indent_width;
};

// ostream that uses IndentStreamBuf
class IndentStream : public std::ostream {
public:
    // Constructor takes an output stream to write to
    IndentStream(std::ostream& output) 
        : std::ostream(nullptr), buffer(output) {
        // Set custom streambuf as the buffer for this stream
        rdbuf(&buffer);
    }
    
    // Provide access to the underlying buffer for manipulators
    IndentStreamBuf* get_buffer() {
        return &buffer;
    }

private:
    IndentStreamBuf buffer;
};

// I/O manipulator to increase indent
std::ostream& indent(std::ostream& os) {
    // Try to cast to IndentStream to access buffer directly
    IndentStream* ins = dynamic_cast<IndentStream*>(&os);
    if (ins) {
        ins->get_buffer()->increase_indent();
    }
    return os;
}

// I/O manipulator to decrease indent
std::ostream& unindent(std::ostream& os) {
    // Try to cast to IndentStream to access buffer directly
    IndentStream* ins = dynamic_cast<IndentStream*>(&os);
    if (ins) {
        ins->get_buffer()->decrease_indent();
    }
    return os;
}

int main() {
    IndentStream ins(std::cout);
    
    ins << "int" << std::endl;
    ins << "fib(int n) {" << indent << std::endl;
    ins << "if (n == 0) return 0;" << std::endl;
    ins << "if (n == 1) return 1;" << std::endl;
    ins << "return fib(n-2) + fib(n-1);" << unindent << std::endl;
    ins << "}" << std::endl;
    
    return 0;
}