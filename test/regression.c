/* regression.c
 *   by Alex Chadwick
 * 
 * Copyright (C) 2014, Alex Chadwick
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>

#include "fsm_test.h"
#include "symbol_test.h"

typedef int (*test_t)(void);

test_t tests[] = {
    FSMTest_Create0,
    FSMTest_Create1,
    FSMTest_Create2,
    FSMTest_Create3,
    FSMTest_Create4,
    FSMTest_Merge0,
    FSMTest_Merge1,
    FSMTest_Run0,
    FSMTest_Run1,
    FSMTest_Run2,
    FSMTest_Run3,
    FSMTest_Run4,
    SymbolTest_Parse0,
    SymbolTest_Parse1,
    SymbolTest_Parse2,
    SymbolTest_Parse3,
};

#define TEST_COUNT (sizeof(tests) / sizeof(*tests))
 
int main(int argc, char *argv[]) {
    int test;
    
    if (argc != 2)
        return 1;
        
    test = atoi(argv[1]);
    
    if (test < 0 || test >= TEST_COUNT)
        return 2;
        
    return tests[test]();
}
