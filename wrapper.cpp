// wrapper.cpp
#include <iostream>

// Объявления оригинальных функций
extern int rc_main(int argc, char* argv[]);
extern int ms_main(int argc, char* argv[]);

// Wrapper для RC
int rc_main_interactive() {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << " REFERENCE COUNTING GARBAGE COLLECTOR" << std::endl;
    std::cout << " Simple RC (No Cycle Detection)" << std::endl;
    std::cout << std::string(80, '=') << "\n" << std::endl;
    
    // Имитация интерактивного запуска
    char program_name[] = "rc_gc";
    char* argv[] = {program_name, nullptr};
    return rc_main(1, argv);
}

// Wrapper для MS
int ms_main_interactive() {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << " MARK-SWEEP GARBAGE COLLECTOR" << std::endl;
    std::cout << " Integrated Test Suite" << std::endl;
    std::cout << std::string(80, '=') << "\n" << std::endl;
    
    // Имитация интерактивного запуска
    char program_name[] = "ms_gc";
    char* argv[] = {program_name, nullptr};
    return ms_main(1, argv);
}