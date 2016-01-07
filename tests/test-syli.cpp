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

  EXPECT_NEAR(2.5, l.firstCIt(pal::dipole)->second->Ecrit_keV_syli(2300./0.511), 0.05);
  EXPECT_NEAR(12.4, l.firstCIt(pal::dipole)->second->meanPhotons_syli(2300./0.511), 0.05);
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
