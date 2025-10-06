/* 
 * S-C (simple C) Optimizer Supports:
 * Basic scalar types (int, long, char, bool)
 * Bitwise operations (&, |, ^, ~, <<, >>)
 * Arithmetic operations (+, -, *, /, %)
 * Boolean operations (&&, ||, !, ==, !=, <, >, <=, >=)
 * Control flow (if, else, while, for (simple bounds), break, continue)
 * Simple (non-recursive) Functions (declaration, definition, calls)
 * Basic arrays and very restrictive pointers
*/

/* Constant Folding: 
 * Form of optimization where an expression can be evaluated by the compiler at compile time, instead of generating code to evaluate it at runtime.
 * ex: 5 + 4 * 5; is the same as x=25; so we can let the compiler evaluate it and just output the ASM for x = 25;
 * To implement, look for sub-trees in an AST tree where the leaves are integer literals. If there is a binary operation which has two integer literals leaves,
 * The compiler can evaluate the expression and replace the sub tree with a single integer literal node.
 * If there is a unary operation with an integer literal leaf child, then the compiler can also evaluate the expression and replace the sub tree with a single integer literal node.
 * Once we can do this for sub-trees, we can write a function to traverse the AST which looks for sub-trees to fold, the algorithm follows as:
 * 1. Try to fold and replace the left child, recursively
 * 2. Try to fold and replace the right child, recursively
 * If its a binary operation with two literals child leaves, fold that.
 * If its a unary operation with one literal child leaf, fold that.
*/

/* Dead-Code:
 * Code that is never executed or a variable that is never used.
 * Other optimizations may create dead code, so this should run last.
 * ex: y = 5; z = 6; x = y + 2; (z is never used, so it can be removed)
*/

/* Loop Optimizations
 * Loop-Invariant Code Motion: If result of a statement or expression does not change inside of a loop, and it has no externally visible side effects (!), you can hoist its 
 * computation outside of the loop.
 * ex: for (i=0; i<100; i++) { x = y; } can be optimized to x = y; for (i=0; i<100; i++) { }
 * Loop Strength Reduction: Replaces an expensive operation (multiply, divide) by cheap ones (add, subtract)
 * ex: for (i=0; i<100; i++) { x = i * 8; } can be optimized to: x = 0; for (i=0; i<100; i++) { x += 8; }
 * Induction Variable Elimination:
 * Loop Unrolling: Execute loop body multiple times per iteration, reducing overhead. Space tradeoff, program size increases.
 * ex: for (i=0;i<n;i++) { S } unrolled 4 times: for (i=0;i<n-3;i+=4) { S; S; S; S; } for (;i<n;i++) { S; }
*/

/* Function Inlining:
 * Replace a function call with the body of the function:
 * ex: int add(int a, int b) { return a + b; } int main() { int x = add(5, 6); } can be optimized to: int main() { int x = 5 + 6; }
 * Reduces function call overhead, enables further optimizations (constant folding, dead code elimination)
*/

/* Optimization Order:
 * 1. Function Inlining
 * 2. Constant Folding
 * 3. Dead Code Elimination
 * 4. Loop Invariant Code Motion
 * 5. Loop Strength Reduction
 * 6. Induction Variable Elimination
 * 7. Loop Unrolling
*/

int main() {

}