$$\newcommand{\bb}[1]{\mathbb{#1}}$$
$$\renewcommand{\bf}[1]{\mathbf{#1}}$$
# Quickstart

# Documentation
## Part A
The simple matrix multiplication method uses a dot product module that performs 16 iterations in order to calculate one sum. We can better it by "unfolding" all 16 iterations in parallel.
	- I considered using fork-join structures to implement the final addition of all the individual products but that would instantiate registers for each of the partial sums which I assumed would ultimately slow it all down.
## Part B
### How does it work?
$$\bf{A}\bf{B} = \begin{bmatrix}\bf{A}_{00} & \bf{A}_{01}\\\bf{A}_{10}& \bf{A}_{11}\end{bmatrix} \begin{bmatrix}\bf{B}_{00} & \bf{B}_{01}\\\bf{B}_{10}& \bf{B}_{11}\end{bmatrix} = \begin{bmatrix}\mathbf{A}_{00}\mathbf{B}_{00} + \bf{A}_{01}\bf{B}_{10} & \mathbf{A}_{00}\mathbf{B}_{01} + \bf{A}_{01}\bf{B}_{11}\\
\mathbf{A}_{10}\mathbf{B}_{00} + \bf{A}_{11}\bf{B}_{10}& \mathbf{A}_{10}\mathbf{B}_{01} + \bf{A}_{11}\bf{B}_{11}\end{bmatrix}$$
Thus dividing the $16\times 16$ matrices into $4$ matrices of sizes $8\times 8$ will result in us having to complete,
- $8$ unique matrix multiplications (between matrices of sizes $8\times 8$)
- 1 Addition per element of the resultant matrix
