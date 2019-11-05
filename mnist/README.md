# MNIST Benchmark

Using the Gravity compiler to build a handwritten digits recognizer from the
MNIST database [http://yann.lecun.com/exdb/mnist/]. The implementation uses
Gravity as a just-in-time compiler. A python3 reference implementation is
provided using tensorflow using exactly the same parameters.

## Obtaining, Building & Running Gravity/MNIST
  1. $ git clone https://github.com/givargis/gravity
  2. $ cd gravity/src
  3. $ make
  4. $ cd ../mnist
  5. $ make
  6. $ ./mnist
  7. $ python3 mnist.py

## Performance

All benchmarks running on:
  * CentOS Linux release 8.0.1905 (Core)
  * 4 G Hz Quad-Core Intel Core i7
  * 32 GB 1867 MHz DDR3
  * Train/Test data preloaded in RAM and excluded from time measurements

```
   Implementation  | Accuracy |  Train Time   |   Test Time
                   |          | (usec/sample) | (usec/sample)
-------------------|----------|---------------|---------------
     Gravity/C     |  0.9670  |     124       |      18
-------------------|--------------------------|---------------
 Python/Tensorflow |  0.9748  |     183       |      39
```
