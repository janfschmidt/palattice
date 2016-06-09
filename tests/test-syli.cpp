#include "gtest/gtest.h"
#include "../SimTools.hpp"
#include "../AccLattice.hpp"


TEST(SyliTests, GSLevolt) {
  EXPECT_DOUBLE_EQ(GSL_CONST_MKSA_ELECTRON_CHARGE, GSL_CONST_MKSA_ELECTRON_VOLT);
}

TEST(SyliTests, Erev) {
  pal::SimToolInstance s(pal::elegant, pal::online, TEST_LATTICE_FILE);
  pal::AccLattice l(s);
  
  EXPECT_NEAR(225., l.Erev_keV_syli(4500.), 1.);
  EXPECT_NEAR(711., l.Erev_keV_syli(6000.), 1.);
}

TEST(SyliTests, syliElement) {
  pal::SimToolInstance s(pal::elegant, pal::online, TEST_LATTICE_FILE);
  pal::AccLattice l(s);

  EXPECT_NEAR(4e-16, l.begin(pal::dipole).element()->syli_Ecrit_Joule(2300./0.511), 8e-18);
  EXPECT_NEAR(2.5, l.begin(pal::dipole).element()->syli_Ecrit_keV(2300./0.511), 0.05);
  EXPECT_NEAR(0.00489, l.begin(pal::dipole).element()->syli_Ecrit_gamma(2300./0.511), 1e-4);
  EXPECT_NEAR(12.4, l.begin(pal::dipole).element()->syli_meanPhotons(2300./0.511), 0.05);
}

TEST(SyliTests, integralRadius) {
  pal::SimToolInstance s(pal::elegant, pal::online, TEST_LATTICE_FILE);
  pal::AccLattice l(s);

  EXPECT_NEAR(11, l.integralDipoleRadius(), 0.05);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
