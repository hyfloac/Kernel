# Calling Conventions

## 32 Bit

### CDecl

Arguments: Passed Right-To-Left on the stack. 

Caller cleaned up.

Return Value: EAX/ST0, structs are stored in a pointer passed in on the stack

Volatile Registers: EAX, ECX, EDX

16 byte aligned on GCC >= 4.5

4 byte aligned on MSVC and GCC < 4.5

### StdCall

Arguments: Passed Right-To-Left on the stack. 

Callee cleaned up.

Return Value: EAX, structs are stored in a pointer passed in on the stack

Volatile Registers: EAX, ECX, EDX

### FastCall

Arguments: First two interger/pointers are passed in ECX and EDX, remainder passed Right-To-Left on the stack. 

Callee cleaned up.

Return Value: EAX, structs are stored in a pointer passed in on the stack

Volatile Registers: EAX, ECX, EDX

### ThisCall

Arguments: this* passed in ECX, remaining passed Right-To-Left on the stack. 

Callee cleaned up.

Return Value: EAX, structs are stored in a pointer passed in on the stack

Volatile Registers: EAX, ECX, EDX

### VectorCall

Arguments: First two interger/pointers are passed in ECX and EDX, First six FP/Vector/HVA arguments are passed in [XY]MM0-5, remainder passed Right-To-Left on the stack. Vectors that could not be passed in the register are passed by reference on the stack.

Callee cleaned up.

Return Value: EAX/[XY]MM, structs/unions of 4 bytes or less are returned in EAX, structs/unions of 4-8 bytes are returned in EDX and EAX. HVA's are passed in [XY]MM0-3

Volatile Registers: EAX, ECX, EDX

