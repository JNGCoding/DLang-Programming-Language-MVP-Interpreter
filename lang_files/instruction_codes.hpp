#ifndef LANGFILES_INSTRUCTION_CODES_HPP__
#define LANGFILES_INSTRUCTION_CODES_HPP__

//^ Anonymous Instructions
//~ Declares the line as a comment 'comment'
#define ICOMMENT "comment"
#define ISHORT_HAND_COMMENT "#"

//~ ends the program right there after reading the line 'endprog'
#define IPROGRAMFINISH "endprog"

//~ Creates a user defined error 'error <error_code> <error_string>'
#define IERROR "error"

//^ Declarations Instructions
//~ Makes a variable with a given type 'make <variable_type> <variable_name>'
#define IMAKE "make"

//~ Makes a function type object 'def <function_type> <function_name>'
#define IDECLARE "def"

//^ Object Set [INPUT] Instructions
//~ Configures the data of a variable with the valid data given 'set <variable_name> <data>'
#define ISET "set"

//~ Takes input from the InputStream defined in the header file 'input <variable_name>'
#define IINPUT "input"

//~ Converts the type of the variable to the given type 'convert <type> <variable_name>'
#define ICONVERT "convert"

//~ Clears the data buffers of a variable 'clearvar <variable_name>'
#define ICLEARVAR "clearvar"

//~ Sets the index of a sequence like variable 'setindex <variable_name>'
#define ISETIDX "setindex"

//~ Gets the index of a sequence like variable and stores it in a variable 'getindex <variable_name>'
#define IGETIDX "getindex"

//^ Object Delete [DEL] Instructions
//~ Deletes the variable from the storage 'del <variable_name>'
#define IDELETE "del"

//~ Deletes the function/linespace from the storage 'undef <function_name/linespace_name>'
#define IDELFUNC "undef"

//^ Object Out [OUTPUT] Instructions
//~ prints a given string to the OutputStream defined in the header file 'print <value>'
#define IPRINT "print"

//~ prints a given string + "\n" to the OutputStream defined in the header file 'println <value>'
#define IPRINTLN "println"

//^ Arithmetic Operator Instructions (Not gonna explain them, but basic instruction structure '<instruction> <variable> <data>' for eg. add numberVariable 102)
#define IADD "add"
#define ISUBTRACT "sub"
#define IDIVIDE "div"
#define IMULTIPLY "mul"
#define IMOD "mod"

//^ Bitwise Operator Instructions (Same as Arithmetic Operators)
#define IOR "or"
#define IAND "and"
#define INOT "not"
#define INOR "nor"
#define INAND "nand"
#define IXOR "xor"
#define ILS "lshift"
#define IRS "rshift"

//^ Function and Linespace Specific functions
//~ Calls a function 'call <function_name>'
#define ICALL "call"

//~ Moves the read pointer to the designated linespace index 'move <linespace_name>'
#define IMOVE "move"

//~ defines the end of a function 'endfunc'
#define IFUNCTIONEND "endfunc"

//^ Conditional Instructions
//~ declares an 'if' condition -- 2 modes = 1) One-liner, 2) Block
//~ 'if <value 1> <operator> <value 2> <define a one line if block by writing another instruction here>'
#define IIF "if"

//~ Ends an if condition (only if declared in 'block' mode) 'endif'
#define IIFEND "endif"

//^ Conditions
#define CEQUAL "eq"
#define CGREATERTHANEQUAL "gteq"
#define CGREATERTHAN "gt"
#define CLOWERTHANEQUAL "lteq"
#define CLOWERTHAN "lt"
#define CNOTEQUAL "neq"

//^ Variable Types <V = Variable, M = Modifier, T = Type>
#define VSTRING_T "string"
#define VNUMBER_T "number"
#define VARRAY_M "array"

//^ Declaration Arguments <F = Function, T = Type>
#define FFUNCTION_T "function"
#define FLINESPACE_T "linespace"

//^ INTERNAL KEYWORDS
#define ALL 0x01
#define GLOBAL 0x02
#define CURRENT 0x03

//^ ERRORS
#define NOT_DEFINED -1
#define BUFFER_OVERFLOW -2
#define TYPE_MISMATCH -3
#define SPECIFIC_ERROR -4
#define INSUFFICIENT_ARGUMENTS -5
#define INDEX_OUT_OF_RANGE -6
#define ALREADY_EXISTS -7
#define MEMORY_CORRUPTION -8
#define PARSE_ERROR -9
#define UNKNOWN_ERROR -32600

//^ INTEGER CODES
#define LINE_PARSED 0
#define PASS 0
#define CONTINUE_LINE -200

#endif