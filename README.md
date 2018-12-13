<h1 align="center">
  <br>
  <a href="https://www.bsc.es/">
    <img src="bsc_logo.png" alt="Barcelona Supercomputing Center" height="60px">
  </a>
  <br>
  <br>
  Thread Affinity for Python
  <br>
</h1>

<h3 align="center">Utility to set thread affinity in python.</h3>

<p align="center">
  <a href="https://travis-ci.org/bsc-wdc/thread_affinity">
    <img src="https://travis-ci.org/bsc-wdc/thread_affinity.svg?branch=master"
         alt="Build Status">
  </a> 
    
</p>


A C++ wrapper that allows to call Linux set &amp; get affinity from Python.

This code is a part of the Python Binding of the COMPSs Programming model developed at BSC, but it can be
installed and used separately.

# Example of usage
## Importing the library
```
>>> import thread_affinity
```
## Setting and getting the affinity of the current process
```
>>> thread_affinity.get_default_affinity()
[0, 1, 2, 3]
>>> thread_affinity.set_affinity([0,3])
>>> thread_affinity.get_affinity()
[0, 3]
```
## Setting and getting the affinity of other processes
```
>>> thread_affinity.get_affinity(4110)
[0, 1, 2, 3]
>>> thread_affinity.set_affinity([0,3], 4110)
>>> thread_affinity.get_affinity(4110)
[0, 3]
>>> # taskset -p 4110 returns 9 (1001)
```
