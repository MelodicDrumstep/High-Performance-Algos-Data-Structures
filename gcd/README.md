# Gcd algorithm

For the optimizations performed, please check out the code and the comments in [gcd.hpp](gcd.hpp).

## Performance Tests

```
Function 'gcd_baseline_recursion' took 232 µs to complete.
Function 'gcd_baseline_loop' took 232 µs to complete.
Function 'gcd_binary' took 498 µs to complete.
Function 'gcd_binary_opt1' took 177 µs to complete.
Function 'gcd_binary_opt2' took 151 µs to complete.
```

## Assembly Code

Assembly code can be found at [here](https://godbolt.org/z/Wr3z7MqY1)

### Recursion & Loop

Actually the recursion version has been optimized to loop verison.

```asm
.LC0:
        .string "basic_string::_M_construct null not valid"

; Recursive GCD implementation using Euclidean algorithm
; Input: edi = a, esi = b
; Output: eax = gcd(a, b)
gcd_baseline_recursion(int, int):
        mov     eax, edi          ; Move a to eax
        mov     r8d, esi          ; Move b to r8d
        test    esi, esi          ; Test if b == 0
        jne     .L28              ; If b != 0, jump to loop
        jmp     .L35              ; Else return a
.L36:
        mov     r8d, edx          ; Move remainder to b
.L28:
        cdq                       ; Sign extend eax to edx:eax
        idiv    r8d               ; Divide eax by r8d (a % b)
        mov     eax, r8d          ; Move b to a
        test    edx, edx          ; Test remainder
        jne     .L36              ; If remainder != 0, continue loop
        mov     eax, r8d          ; Return gcd
        ret
.L35:
        mov     r8d, edi          ; Return original a when b == 0
        mov     eax, r8d
        ret

; Iterative GCD implementation using Euclidean algorithm
; Input: edi = a, esi = b
; Output: eax = gcd(a, b)
gcd_baseline_loop(int, int):
        mov     eax, edi          ; Move a to eax
        mov     edx, esi          ; Move b to edx
        test    esi, esi          ; Test if b <= 0
        jle     .L40              ; If b <= 0, return a
.L39:
        mov     r8d, edx          ; Move b to r8d
        cdq                       ; Sign extend eax to edx:eax
        idiv    r8d               ; a % b
        mov     eax, r8d          ; b becomes new a
        test    edx, edx          ; Test remainder
        jg      .L39              ; If remainder > 0, continue loop
        mov     eax, r8d          ; Return gcd
        ret
.L40:
        mov     r8d, edi          ; Return original a when b <= 0
        mov     eax, r8d
        ret
```

### Gcd Binary

```asm
; Binary GCD algorithm implementation
; Uses properties of even/odd numbers to avoid modulo operations
; Input: edi = a, esi = b
; Output: eax = gcd(a, b)
gcd_binary(int, int):
        mov     eax, esi          ; Move b to eax
        mov     esi, 1            ; Initialize shift counter
.L43:
        test    edi, edi          ; Check if a == 0
        je      .L44              ; If a == 0, return b*shift
.L52:
        cmp     edi, eax          ; Compare a and b
        je      .L48              ; If equal, return a*shift
        test    eax, eax          ; Check if b == 0
        je      .L48              ; If b == 0, return a*shift
        mov     edx, eax          ; Check if b is odd
        and     edx, 1
        test    dil, 1            ; Check if a is odd
        jne     .L45              ; If a is odd, jump
        ; Case when a is even
        mov     ecx, edi
        shr     ecx, 31           ; Arithmetic shift right
        add     edi, ecx
        sar     edi               ; a /= 2
        test    edx, edx          ; Check if b is odd
        jne     .L43              ; If b is odd, continue
        ; Case when both a and b are even
        mov     edx, eax
        add     esi, esi          ; Double the shift counter
        shr     edx, 31           ; Arithmetic shift right
        add     eax, edx
        sar     eax               ; b /= 2
        test    edi, edi          ; Check if a == 0
        jne     .L52              ; Continue if not zero
.L44:
        imul    eax, esi          ; Multiply result by 2^shift
        ret
.L45:
        test    edx, edx          ; Check if b is odd
        je      .L53              ; If b is even, handle
        ; Case when both a and b are odd
        cmp     edi, eax          ; Compare a and b
        mov     edx, eax
        cmovle  edx, edi          ; edx = min(a, b)
        sub     edi, eax          ; a = a - b
        mov     eax, edi
        sar     eax, 31           ; Absolute value
        xor     edi, eax
        sub     edi, eax
        mov     eax, edx          ; b = min(original a, b)
        jmp     .L43              ; Continue loop
.L48:
        mov     eax, edi          ; Return a*shift
        imul    eax, esi
        ret
.L53:
        ; Case when a is odd, b is even
        mov     edx, eax
        shr     edx, 31           ; Arithmetic shift right
        add     eax, edx
        sar     eax               ; b /= 2
        jmp     .L43              ; Continue loop
```

The problem is —— too many branches!

### Gcd Binary Optimization : Smart Observations

```asm
; Optimized binary GCD version 1
; Uses __builtin_ctz for counting trailing zeros
; Input: edi = a, esi = b
; Output: eax = gcd(a, b)
gcd_binary_opt1(int, int):
        mov     eax, esi          ; Move b to eax
        test    edi, edi          ; Check if a == 0
        je      .L54              ; Return b if a == 0
        mov     eax, edi          ; Move a to eax
        test    esi, esi          ; Check if b == 0
        je      .L54              ; Return a if b == 0
        xor     ecx, ecx
        xor     edx, edx
        rep bsf ecx, edi          ; Count trailing zeros in a
        rep bsf edx, esi          ; Count trailing zeros in b
        cmp     ecx, edx          ; Find minimum shift count
        mov     r8d, edx
        cmovle  r8d, ecx
        sar     eax, cl           ; Remove trailing zeros from a
        mov     ecx, edx
        sar     esi, cl           ; Remove trailing zeros from b
        test    eax, eax          ; Check if a == 0
        je      .L56              ; If a == 0, return b << shift
.L57:
        mov     edi, eax
        sub     edi, esi          ; diff = a - b
        cmp     esi, eax          ; Find min(a, b)
        cmovg   esi, eax
        mov     eax, edi          ; Prepare for absolute value
        xor     ecx, ecx
        sar     eax, 31           ; Compute absolute value
        xor     edi, eax
        sub     edi, eax          ; a = abs(diff)
        rep bsf ecx, edi          ; Count trailing zeros in a
        mov     eax, edi
        sar     eax, cl           ; Remove trailing zeros
        test    eax, eax          ; Check if a == 0
        jne     .L57              ; Continue loop if not zero
.L56:
        mov     eax, esi          ; Return b << shift
        mov     ecx, r8d
        sal     eax, cl
.L54:
        ret

; Optimized binary GCD version 2
; Further optimization by using diff for trailing zero count
; Input: edi = a, esi = b
; Output: eax = gcd(a, b)
gcd_binary_opt2(int, int):
        mov     eax, esi          ; Move b to eax
        test    edi, edi          ; Check if a == 0
        je      .L66              ; Return b if a == 0
        mov     eax, edi          ; Move a to eax
        test    esi, esi          ; Check if b == 0
        je      .L66              ; Return a if b == 0
        xor     ecx, ecx
        xor     edx, edx
        rep bsf ecx, edi          ; Count trailing zeros in a
        rep bsf edx, esi          ; Count trailing zeros in b
        cmp     ecx, edx          ; Find minimum shift count
        mov     r8d, edx
        cmovle  r8d, ecx
        sar     eax, cl           ; Remove trailing zeros from a
        mov     ecx, edx
        sar     esi, cl           ; Remove trailing zeros from b
        test    eax, eax          ; Check if a == 0
        je      .L68              ; If a == 0, return b << shift
.L69:
        mov     edi, eax
        sub     edi, esi          ; diff = a - b
        cmp     esi, eax          ; Find min(a, b)
        cmovg   esi, eax
        mov     eax, edi          ; Prepare for absolute value
        xor     ecx, ecx
        sar     eax, 31           ; Compute absolute value
        rep bsf ecx, edi          ; Count trailing zeros in diff
        xor     edi, eax
        sub     edi, eax
        mov     eax, edi
        sar     eax, cl           ; Remove trailing zeros
        test    eax, eax          ; Check if a == 0
        jne     .L69              ; Continue loop if not zero
.L68:
        mov     eax, esi          ; Return b << shift
        mov     ecx, r8d
        sal     eax, cl
.L66:
        ret
```

Notice that

```asm
        rep bsf ecx, edi          ; Count trailing zeros in a
        rep bsf edx, esi          ; Count trailing zeros in b
```

gives us potential performance benifit because it can be executed as `tzcnt` instruction on some machines. Actually if we disassemble the executable, we will see

```asm
    1fe4:	f3 0f bc cf          	tzcnt  %edi,%ecx
```

And we can see that after using `a >>= __builtin_ctz(diff);` rather than `a >>= __builtin_ctz(a);` in `gcd_binary_opt2`, the data hazard is eliminated. And the compiler actually reorder "computing __builtin_ctz(diff)" before "a = std::abs(diff)" for better performance. 

Here we can see the power for eliminating data hazard in the CPU pipeline.

## Reference

[en.algorithmica.org](https://en.algorithmica.org/hpc/algorithms/gcd/)

[x86 assembly](https://web.stanford.edu/class/cs107/guide/x86-64.html)

[x86 assembly cheat sheet](https://web.stanford.edu/class/archive/cs/cs107/cs107.1256/resources/x86-64-reference.pdf)