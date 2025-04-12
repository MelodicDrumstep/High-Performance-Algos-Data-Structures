# Division Operation

## Performance Tests

For inputs in $[1, 100000]$:

```
Function 'division_baseline' took 18 µs to complete.
Function 'division_baseline2' took 18 µs to complete.
Function 'division_Barrett_reduction' took 30 µs to complete.
Function 'division_Lemire_reduction' took 31 µs to complete.
Function 'division_baseline' (Precomputing Version) took 18 µs to complete.
Function 'division_baseline2' (Precomputing Version) took 18 µs to complete.
Function 'division_Barrett_reduction' (Precomputing Version) took 13 µs to complete.
Function 'division_Lemire_reduction' (Precomputing Version) took 22 µs to complete.
```

## Assembly Code

### Baseline

In fact, the two versions of the baseline source code result in the same assembly code. 

```asm
division_baseline(unsigned int, unsigned int):
        ; Input: a = edi (1st arg), b = esi (2nd arg)
        ; Output: quotient in eax, remainder in edx (packed into rax)

        mov     eax, edi      ; Load dividend (a) into eax (for div instruction)
        xor     edx, edx      ; Zero out edx (upper 32 bits of dividend, since it's 32-bit division)
        div     esi           ; Unsigned division: eax = eax / esi (quotient), edx = remainder
        sal     rdx, 32       ; Shift remainder (edx) left by 32 bits (to store in upper half of rax)
        or      rax, rdx      ; Combine quotient (eax) and shifted remainder (rdx) into rax
        ret                   ; Return (rax holds the packed DivResult)

division_baseline2(unsigned int, unsigned int):
        ; The same
```

### Barrett Reduction

```asm
division_Barrett_reduction(unsigned int, unsigned int):
        ; Input: a (dividend) in edi, b (divisor) in esi
        ; Output: quotient in eax, remainder in edi (packed into xmm0)

        mov     ecx, esi      ; Move divisor (b) to ecx for division
        mov     rax, -1       ; Load 2^64 - 1 into rax (max uint64_t value)
        xor     edx, edx      ; Clear edx (upper bits for 64-bit division)
        div     rcx           ; Divide rax by rcx: rax = floor(2^64 / b), rdx = remainder
        
        ; Now rax contains m = floor(2^64 / b)
        mov     edx, edi      ; Move dividend (a) to edx
        mov     ecx, esi      ; Move divisor (b) to ecx again
        mul     rdx           ; Multiply m (in rax) by a (in rdx), result in rdx:rax
                              ; Since we only care about the high 64 bits (rdx), we ignore rax
        
        ; Compute quotient approximation (q = floor((a*m)/2^64))
        imul    ecx, edx      ; Compute q*b (quotient * divisor)
        mov     eax, edx      ; Move quotient to eax (return value)
        sub     edi, ecx      ; Compute remainder: r = a - q*b
        
        ; Adjustment step (if remainder >= b)
        cmp     esi, edi      ; Compare divisor (b) with remainder
        ja      .L6           ; If b > remainder (r < b), jump to .L6 (no adjustment)
        lea     eax, [rdx+1]  ; Else increment quotient (q += 1)
        sub     edi, esi      ; And adjust remainder (r -= b)
        
.L6:
        ; Pack quotient and remainder into xmm0 for return
        movd    xmm0, eax     ; Move quotient to xmm0[31:0]
        movd    xmm1, edi     ; Move remainder to xmm1[31:0]
        punpckldq xmm0, xmm1  ; Pack both 32-bit values into xmm0[63:0]
        movq    rax, xmm0     ; Move packed result to rax for return
        ret                   ; Return (rax contains {quotient, remainder})
```

### Lemire Reduction

```asm
division_Lemire_reduction(unsigned int, unsigned int):
        ; Input: a (dividend) in edi, b (divisor) in esi
        ; Output: quotient in r9d, remainder in edx (packed into xmm0)

        mov     ecx, esi      ; Move divisor (b) to ecx
        xor     edx, edx      ; Clear edx (upper bits for division)
        mov     rax, -1       ; Load 2^64 - 1 into rax
        mov     edi, edi      ; Zero-extend a to 64 bits (clears upper 32 bits of rdi)
        div     rcx           ; Compute m = floor(2^64 / b)
        
        ; Lemire computes m = ceil(2^64 / b) = floor(2^64 / b) + 1
        lea     rsi, [rax+1]  ; m = rax + 1 (Lemire's adjustment)
        
        ; Compute quotient = floor((a * m) / 2^64)
        mov     rax, rdi      ; Move dividend (a) to rax
        mul     rsi           ; Multiply a by m, result in rdx:rax
        mov     rax, rdi      ; Reload a (needed for remainder calculation)
        imul    rax, rsi      ; Compute a * m again (could be optimized out)
        mov     r9, rdx       ; Save quotient (high bits of product) in r9
        
        ; Compute remainder = floor((a * m * b) / 2^64)
        mul     rcx           ; Multiply previous result (a*m) by b, result in rdx:rax
                              ; We only need the high bits (rdx) which is the remainder
        
        ; Pack results for return
        movd    xmm0, r9d     ; Move quotient to xmm0[31:0]
        movd    xmm1, edx     ; Move remainder to xmm1[31:0]
        punpckldq xmm0, xmm1  ; Pack both values into xmm0[63:0]
        movq    rax, xmm0     ; Move packed result to rax for return
        ret                   ; Return (rax contains {quotient, remainder})
```