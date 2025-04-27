$$\newcommand{\bb}[1]{\mathbb{#1}}$$
$\newcommand{\t}[1]{\text{#1}}$
$$\renewcommand{\bf}[1]{\mathbf{#1}}$$
# Quickstart
To build part `x` (`x` can be either `a`, `b` or `c`), run
```sh
make partb # Note that you can replace 'partb' with any of the following {no_unrolling, partial_unrolling, parta, partb, partc}
```
After that run `tmux new` and run the following in two different panes
```sh
# First pane (Run this first)
./testbench_hw
```
```sh
# Second pane (Run this after the first one)
./ahir_system_test_bench
```
- The testbench will execute the matrix multiplication in C and compare the results it gets from the Aa modules. In case of any discrepancies, it will print them on screen. 
- It will print the time taken for matrix multiplication (both including and excluding the time taken by the function call).
# Documentation
## Miscellaneous
### System Specs
Any and all timing values mentioned depend on the specifications of the system in use
```
Architecture:             x86_64
  CPU op-mode(s):         32-bit, 64-bit
  Address sizes:          39 bits physical, 48 bits virtual
  Byte Order:             Little Endian
CPU(s):                   8
  On-line CPU(s) list:    0-7
Vendor ID:                GenuineIntel
  Model name:             11th Gen Intel(R) Core(TM) i5-11300H @ 3.10GHz
    CPU family:           6
    Model:                140
    Thread(s) per core:   2
    Core(s) per socket:   4
    Socket(s):            1
    Stepping:             1
    CPU(s) scaling MHz:   18%
    CPU max MHz:          4400.0000
    CPU min MHz:          400.0000
```
### How to measure timing
Since there might be some time spent in the function call itself, I created a module called `latencyTest` which is as follows,
```aa
$module [latencyTest] $in () $out () $is // This takes no inputs, returns no outputs, does nothing.
{
	$null
}
```
- Using the `clock()` function in C will be wrong since it doesn't measure wall-clock time -- rather it looks at the time on the processor.
- Thus I used `clock_gettime()` which promises to be more faithful to the wall-clock time.
- Generally `latencyTest` took $\approx 30\t{ ms}$.
- All timing values reported hereafter are after subtracting time taken by `latencyTest`.

**The native (without unrolling) dot product took $\approx 80\t{ s}$.**
## Part A : Using a better dot product
The simple matrix multiplication method uses a dot product module that performs 16 iterations in order to calculate one sum. We can better it by "unfolding" all 16 iterations in parallel.
- I considered using fork-join structures to implement the final addition of all the individual products but that would instantiate registers for each of the partial sums which I assumed would ultimately slow it all down.

Thus the final module for dot_product I used is,
```aa
$module [dot_product_fully_unrolled] $in (I J: $uint<8>) $out (result: $uint<32>) $is
{
	// This is tailored for ORDER = 16. There is a lot of parallelism thus.
	$parallelblock [p1]
	{
		prod0 := (A[I][0] * B[0][J])
		prod1 := (A[I][1] * B[1][J])
		prod2 := (A[I][2] * B[2][J])
		prod3 := (A[I][3] * B[3][J])
		prod4 := (A[I][4] * B[4][J])
		prod5 := (A[I][5] * B[5][J])
		prod6 := (A[I][6] * B[6][J])
		prod7 := (A[I][7] * B[7][J])
		prod8 := (A[I][8] * B[8][J])
		prod9 := (A[I][9] * B[9][J])
		prod10 := (A[I][10] * B[10][J])
		prod11 := (A[I][11] * B[11][J])
		prod12 := (A[I][12] * B[12][J])
		prod13 := (A[I][13] * B[13][J])
		prod14 := (A[I][14] * B[14][J])
		prod15 := (A[I][15] * B[15][J])
	}(  prod0 => prod0
		prod1 => prod1
		prod2 => prod2
		prod3 => prod3
		prod4 => prod4
		prod5 => prod5
		prod6 => prod6
		prod7 => prod7
		prod8 => prod8
		prod9 => prod9
		prod10 => prod10
		prod11 => prod11
		prod12 => prod12
		prod13 => prod13
		prod14 => prod14
		prod15 => prod15)
	$volatile result := (
		( ((prod1 + prod2) + (prod3 + prod4)) + ((prod5 + prod6) + (prod7 + prod8)) ) 
		+
		( ((prod9 + prod10) + (prod11 + prod12)) + ((prod13 + prod14) + (prod15 + prod0)) )
		)
}
```

**Matrix multiplication with a fully unrolled multiplication took $\approx260 \t{ ms}$.**

### Pipelining : To be or not to be?
The `mmul` module is pipelined in the original case. The above module for dot-product computation isn't pipelineable. Thus is it better to keep the dot product pipelineable itself?
- By using an unrolled version of the dot product module with a parallelism of just 8, we get a runtime of $560 \t{ ms}$. This is double of what we get when we use 16x parallelism. Thus, it is better to just stick to the above module.
## Part B : Dividing matrix into sub-matrices
### How does it bring speedup?
$$\bf{A}\bf{B} = \begin{bmatrix}\bf{A}_{00} & \bf{A}_{01}\\\bf{A}_{10}& \bf{A}_{11}\end{bmatrix} \begin{bmatrix}\bf{B}_{00} & \bf{B}_{01}\\\bf{B}_{10}& \bf{B}_{11}\end{bmatrix} = \begin{bmatrix}\mathbf{A}_{00}\mathbf{B}_{00} + \bf{A}_{01}\bf{B}_{10} & \mathbf{A}_{00}\mathbf{B}_{01} + \bf{A}_{01}\bf{B}_{11}\\
\mathbf{A}_{10}\mathbf{B}_{00} + \bf{A}_{11}\bf{B}_{10}& \mathbf{A}_{10}\mathbf{B}_{01} + \bf{A}_{11}\bf{B}_{11}\end{bmatrix}$$
Thus dividing the $16\times 16$ matrices into $4$ matrices of sizes $8\times 8$ will result in us having to complete,
- $8$ unique matrix multiplications (between matrices of sizes $8\times 8$)
- 1 Addition per element of the resultant matrix
We can bring in a lot of parallelism.
### Implementation
Thus in order to implement it, I have defined $\bf{C}_\t{temp}$ as
$$\bf{C}_\t{temp} = \begin{bmatrix}
\mathbf{A}_{00}\mathbf{B}_{00} & \mathbf{A}_{00}\mathbf{B}_{01} & \mathbf{A}_{10}\mathbf{B}_{00} & \mathbf{A}_{10}\mathbf{B}_{01} & \mathbf{A}_{01}\mathbf{B}_{10} & \mathbf{A}_{01}\mathbf{B}_{11} & \mathbf{A}_{11}\mathbf{B}_{10} & \mathbf{A}_{11}\mathbf{B}_{11}
\end{bmatrix}$$
#### Fully parallelised architecture
One implementation of calculating $\bf{C}_\t{temp}$ can be
```pseudo
parallel{
	compute C_temp[0]
	compute C_temp[1]
	compute C_temp[2]
	compute C_temp[3]
	compute C_temp[4]
	compute C_temp[5]
	compute C_temp[6]
	compute C_temp[7]
}
parallel{
	C_temp[0] = C_temp[0] + C_temp[4]
	C_temp[1] = C_temp[1] + C_temp[5]
	C_temp[2] = C_temp[2] + C_temp[6]
	C_temp[3] = C_temp[3] + C_temp[7]
}
// Thus now C_temp[0] => top-left, C_temp[1] => top-right, C_temp[2] => bottom-left, C_temp[3] => bottom-right
```
- Note that in order to actually get the speedup we want, the module we call for computing elements of `C_temp` (called `mmul_8` in my implementation), we will need to specify it with the `$operator` keyword so a new copy is substituted at every relevant point.
#### Pipelined architecture
Another implementation of finding $\bf{C}_\t{temp}$ can be
```
i = 0
do-pipeline $depth 100 $fullrate{ // we want to exploit as much of the parallelism as possible 
	compute C_temp[i]
	i++
}
parallel{
	C_temp[0] = C_temp[0] + C_temp[4]
	C_temp[1] = C_temp[1] + C_temp[5]
	C_temp[2] = C_temp[2] + C_temp[6]
	C_temp[3] = C_temp[3] + C_temp[7]
}
// Thus now C_temp[0] => top-left, C_temp[1] => top-right, C_temp[2] => bottom-left, C_temp[3] => bottom-right
```
The pipelined architecture is clearly going to be slower but less expensive compared to the fully parallelised one. *In the end, I deleted the pipelined version in favour of using the non-pipelined one.*
## Part C : Summing up Rank-1 Matrices
									Similar to Part B this can be achieved by using multiple modules in parallel.

## Final Comparison

| Type                                            | Time taken (in s) | Speedup |
| ----------------------------------------------- | ----------------- | ------- |
| Native matrix multiplication                    | 80                | 1       |
| Partially unrolled matrix multiplication[^1]    | 54                | 1.48    |
| *Part A* : Fully unrolled matrix multiplication | 21                | 3.80    |
| *Part B* : Blocking multiplication              | 42                | 1.90    |
| *Part C* : Summation of Rank-1 matrices         | -                 | -       |

[^1]: This version cannot be built directly. You need to comment and uncomment relevant sections of `mmul.aa` and then build using `make original`. 
