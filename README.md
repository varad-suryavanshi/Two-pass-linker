### ✅ Final `README.md` for Your Repo:

# 🧠 Two-Pass Linker – OS Lab 1

This project implements a **Two-Pass Linker** in C, as part of the Operating Systems course. It processes an input file that represents modules with symbol definitions, use lists, and instruction sets, and generates a **symbol table** and **memory map**, handling errors and warnings along the way.

---

## 🗃️ Project Structure

```
.
├── lab1_code.c         # Main C source file
├── Makefile            # For compiling the program
├── README.md           # This file
├── Inputs/             # Sample inputs and scripts
│   ├── input-01 to 20  # Sample input files
│   ├── runit.sh        # Runs the linker on all inputs
│   └── gradeit.sh      # Compares output to reference
├── output/             # Reference output files (for grading)
└── .gitignore          # Ignores binaries, logs, etc.
```

---

## ⚙️ Compilation

Make sure you're in the project directory and simply run:

```bash
make
```

This compiles `lab1_code.c` into an executable named `linker`.

---

## 🚀 Running the Linker Manually

```bash
./linker Inputs/input-01
```

It will print the symbol table and memory map to the terminal.

---

## 🤖 Automating Tests with `runit.sh`

To run the linker on **all 20 input files automatically** and store outputs:

```bash
bash Inputs/runit.sh myoutputs ./linker
```

- `myoutputs/` will be created (if not already) to store outputs like `out-1`, `out-2`, etc.
- Make sure `input-01` to `input-20` are in `Inputs/`.

---

## 🧪 Grading with `gradeit.sh`

To compare your outputs against the reference outputs in `output/`:

```bash
bash Inputs/gradeit.sh myoutputs output/
```

This generates a `LOG.txt` summarizing the diff between your results and the reference.

---

## 🧠 About the Two-Pass Linker

- **Pass 1**:
  - Builds the **symbol table**
  - Handles multiple definition warnings
  - Validates instruction counts

- **Pass 2**:
  - Generates the **memory map**
  - Resolves symbols using use lists
  - Handles all **addressing modes**: M, A, R, I, E
  - Detects undefined symbols, relative/absolute overflows, and more

All parse errors and runtime warnings follow the formats provided in the assignment spec.

---

## 📊 Sample Output

```
Symbol Table
foo=5
bar=9 Error: This variable is multiple times defined; first value used

Memory Map
000: 1004
001: 5678 Error: Relative address exceeds module size; relative zero used
...
```

---

## ✅ Scoring

This implementation scored **100/100** using the provided grading scripts.

---

## 👨‍💻 Author

Varad Suryavanshi  
[GitHub Profile](https://github.com/varad-suryavanshi)

---

## 📝 License

This project is part of coursework and is for educational use only.

---
