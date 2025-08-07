# semi-magic-squares-screener

# Operating Systems - UNICT - A.Y. 2024/25

## Project: semi-magic squares screener

### Description

Create a C program that accepts command-line invocations of the following format:

binary &lt;M-square-size&gt; &lt;bin-file-1&gt; … &lt;bin-file-N&gt;

The program takes as input **N ≥ 1** binary files containing representations of **M×M** square matrices of numbers, with **3 ≤ M ≤ 16**, and filters out those that represent **semi-magic squares**.  
A matrix is considered a **semi-magic square** if **all its rows and columns** (but **not** its diagonals) contain numbers that sum to the same value (called the **semi-magic total**).

### Program Requirements

At startup, the program must create **N + 1 threads**:

- **N reader threads**, each responsible for reading one of the specified binary files.  
  - Each file is composed of binary records of **M×M bytes**, representing the elements of each row of the matrix.  
  - Files must be read **using memory mapping (`mmap`)**; any other method is considered incorrect.  
  - Example files: `semi-squares-bin.zip`

- **1 verifier thread**, responsible for checking whether each given **M×M** matrix is a semi-magic square.

### Shared Data Structures

- An **intermediate queue** with a maximum capacity of **5 elements**, containing matrices read from files that need verification.
- A **final record** containing exactly **one matrix**, used to pass verified semi-magic squares to the main thread.
- **Mutexes and condition variables**. The exact number and usage pattern are to be determined by the student, with a minimal design.
- Optional **flags or work counters** for completion signaling.

### Thread Behavior

- All **reader threads** operate in parallel, reading the matrices from their respective files.  
  For each matrix to verify, a record is created and inserted into the **intermediate queue**.

- The **verifier thread**:
  - Extracts matrices from the intermediate queue one at a time.
  - Checks whether each matrix is semi-magic.
  - If valid, places the matrix into the **final record**.

- The **main (parent) thread**:
  - Retrieves semi-magic squares from the final record.
  - Displays the verified matrices.