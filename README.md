# DLang-Programming-Language-MVP-Interpreter
This is an VM/Interpreter for my interpreted language but without any optimizations  

demo example  
1)  
make string X "Hello, World"
println $X

output: Hello, World

2)  
make string name
input name
println "Hello " $X ", How are you?"

output : Hello <name>, How are you?

3)  
make number X
make number Y

print "Input X: "
input X

print "Input Y: "
input Y

add X $Y
println "Sum: " X

output:
Input X: <your-input>
Input Y: <your-input>
Sum: <X + Y>
