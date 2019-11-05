# MNIST Benchmark

Using the Gravity compiler to build a handwritten digits recognizer from the
MNIST database [http://yann.lecun.com/exdb/mnist/]. The implementation uses
Gravity as a just-in-time compiler. A python3 reference implementation is
provided using tensorflow and identical model/hyper-parameters.

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
  * Train/Test data preloaded in RAM and excluded from time measurements
  * 4 GHz Quad-Core Intel Core i7
  * 32 GB 1867 MHz DDR3
  * CentOS Linux release 8.0.1905 (Core)
  * GCC 8.2.1
  * Python 3.6.8
  * Tensorflow 2.0.0

```
   Implementation  | Accuracy |  Train Time   |   Test Time   |   Mememory
                   |          | (usec/sample) | (usec/sample) |     (MB)
                   |          |               |               |-------|--------
		   |          |               |               | Train | Activ.
-------------------|----------|---------------|---------------|-------|--------
     Gravity/C     |  0.9670  |     124       |      18       | 2.887 | 1.434
-------------------|--------------------------|---------------|----------------
 Python/Tensorflow |  0.9707  |     154       |      39       |      209*

* measuring libtensorflow_framework.so.2 resident memory only
```
