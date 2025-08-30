#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "=== Running SearchEngine Tests ===" << std::endl;
    
    // Инициализация Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Настройка вывода тестов
    ::testing::FLAGS_gtest_color = "yes";
    ::testing::FLAGS_gtest_brief = false;
    
    std::cout << "\nStarting test execution..." << std::endl;
    
    // Запуск всех тестов
    int result = RUN_ALL_TESTS();
    
    std::cout << "\n=== Test Summary ===" << std::endl;
    if (result == 0) {
        std::cout << "All tests PASSED! ✓" << std::endl;
    } else {
        std::cout << "Some tests FAILED! ✗" << std::endl;
    }
    
    return result;
}