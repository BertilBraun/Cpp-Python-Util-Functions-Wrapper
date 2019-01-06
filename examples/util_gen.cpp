#include <iostream>
#include <vector>
#include <list>

#include <util/generator.h>

int main() {
    std::cout << "Beginning examples: util::gen" << std::endl << std::endl;

    {
        std::cout << "range" << std::endl;

        std::cout << "Should print (0)(1)(2)(3)(4)" << std::endl << "             ";
        for(int64_t value : util::gen::range(5)) {
            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;

        std::cout << "Should print (3)(4)(5)(6)" << std::endl << "             ";
        for(int64_t value : util::gen::range(3,7)) {
            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;

        std::cout << "Should print (2)(5)(8)" << std::endl << "             ";
        for(int64_t value : util::gen::range(2,11,3)) {
            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    {
        std::cout << "enumerate" << std::endl;
        std::vector<int> vec{1,2,3};

        std::cout << "Should print (0,1)(1,2)(2,3)" << std::endl << "             ";
        for(auto&& [index, value] : util::gen::enumerate{vec}) {
            std::cout << "(" << index << "," << value << ")";
        }
        std::cout << std::endl;

        std::cout << "Should print (10,1)(11,2)(12,3)" << std::endl << "             ";
        for(auto&& [index, value] : util::gen::enumerate{vec, 10}) {
            std::cout << "(" << index << "," << value << ")";
        }
        std::cout << std::endl;

        auto get_vec = [](){
            return std::vector<int>{1,2,3};
        };
        std::cout << "Should print (0,1)(1,2)(2,3)" << std::endl << "             ";
        for(auto&& [index, value] : util::gen::enumerate{get_vec()}) {
            std::cout << "(" << index << "," << value << ")";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    {
        std::cout << "zip" << std::endl;
        std::vector<int> vec1{1,2,3};
        std::vector<int> vec2{4,5,6};
        std::list<double> list1{1.2, 3.4, 5.6};
        [[maybe_unused]] char arr[] = {'a', 'b', 'c'};

        std::cout << "Should print (1)(2)(3)" << std::endl << "             ";
        for(auto&& [vec1_val] : util::gen::zip{vec1}) {
            std::cout << "(" << vec1_val << ")";
        }
        std::cout << std::endl;

        std::cout << "Should print (1,4)(2,5)(3,6)" << std::endl << "             ";
        for(auto&& [vec1_val, vec2_val] : util::gen::zip{vec1, vec2}) {
            std::cout << "(" << vec1_val << "," << vec2_val << ")";
        }
        std::cout << std::endl;

        std::cout << "Should print (1,4,1.2)(2,5,2.3)(3,6,4.5)" << std::endl << "             ";
        for(auto&& [vec1_val, vec2_val, list1_val] : util::gen::zip{vec1, vec2, list1}) {
            std::cout << "(" << vec1_val << "," << vec2_val << "," << list1_val << ")";
        }
        std::cout << std::endl;

        std::cout << "Should print (1,4,a)(2,5,b)(3,6,c)" << std::endl << "             ";
        for(auto&& [vec1_val, vec2_val, arr_val] : util::gen::zip{vec1, vec2, arr}) {
            std::cout << "(" << vec1_val << "," << vec2_val << "," << arr_val << ")";
        }
        std::cout << std::endl;

        auto get_vec = [](){
            return std::vector<int>{7,8,9};
        };
        std::cout << "Should print (1,7,a)(2,8,b)(3,9,c)" << std::endl << "             ";
        for(auto&& [vec1_val, vec2_val, arr_val] : util::gen::zip{vec1, get_vec(), arr}) {
            std::cout << "(" << vec1_val << "," << vec2_val << "," << arr_val << ")";
        }
        std::cout << std::endl;
    }

    return 0;
}