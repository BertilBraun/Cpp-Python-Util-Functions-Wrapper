# C++ implementation of Python utilities

 Forked from nightwolf55la - Thank you very much!

## Includes
All of these work just like in Python
- zip
     ```c++
     for (auto&& [vec1_val, vec2_val] : zip{ vec1, vec2 }) {
     }
     ```
- enumerate
     ```c++
     for (auto&& [index, value] : enumerate{ vec }) {
     }
     ```
- range
     ```c++
     for (int64_t i : range(5)) {
     }
     ```
 