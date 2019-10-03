#include <gtest/gtest.h>

int main(int argc, const char **argv)
{
  // banner
  std::cout << "Running SampleTest C++ unit tests.\n";

  /*
   * first, initiate Google Test framework - this will remove
   * framework-specific parameters from argc and argv
   */
  ::testing::InitGoogleTest(&argc, const_cast<char**>(argv));
  int rc = RUN_ALL_TESTS();

  return rc;
}
