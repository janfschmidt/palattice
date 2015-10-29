#include "gtest/gtest.h"
#include "../FunctionOfPos.hpp"
#include "SDDS.h"




TEST(sddsFoP, OrbitTest) {
  pal::SimToolInstance elegant(pal::elegant, pal::online, "elsa.lte");
  elegant.SDDS = true;
  
  pal::FunctionOfPos<pal::AccPair> orbit(elegant);
  orbit.simToolClosedOrbit(elegant);
  orbit.print("sdds-orbit.dat");
}

TEST(sdds, Parameter) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.twi")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetParameter(t, const_cast<char*>("pCentral"), NULL);
  if (mem == NULL)
    throw sddsi::SDDSFailure();
  
  double p = *static_cast<double *>(mem);
  EXPECT_NEAR(4.500987e+03, p, 0.001);
}

TEST(sdds, Column) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.clo")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetColumn(t, const_cast<char*>("s"));
  if (mem == NULL)
    throw sddsi::SDDSFailure();

  double *array = static_cast<double *>(mem);
  unsigned int length=SDDS_CountRowsOfInterest(t); 
  std::vector<double> x(array, array+length);

  for (auto &val : x) {
    std::cout << val << std::endl;
  }
  //EXPECT_NEAR(4.500987e+03, p, 0.001);
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
