#include "gtest/gtest.h"
#include "../SimTools.hpp"

#include <sstream>

double fn(double x) {return x +1.;}

TEST(EnergyRampTest, setFunction) {
  pal::EnergyRamp myramp(fn);  
  EXPECT_DOUBLE_EQ(1., myramp.get(0.));
  EXPECT_DOUBLE_EQ(43.7, myramp.get(42.7));

  pal::EnergyRamp mylambdaramp([&](double x){return x+1.;});
  EXPECT_DOUBLE_EQ(1., mylambdaramp.get(0.));
  EXPECT_DOUBLE_EQ(43.7, mylambdaramp.get(42.7));

  mylambdaramp.set([&](double x){return x*x;});
  EXPECT_DOUBLE_EQ(9., mylambdaramp.get(3.));
}

TEST(EnergyRampTest, toFile) {
  pal::EnergyRamp mylambdaramp([&](double x){return x+1.;});
  mylambdaramp.toFile("ramp.sdds");
}




int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

