#include "gtest/gtest.h"
#include "../FunctionOfPos.hpp"
#include "SDDS.h"




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
  delete t;
}

TEST(sdds, StringParameter) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice002.w")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetParameter(t, const_cast<char*>("PreviousElementName"), NULL);
  if (mem == NULL)
    throw sddsi::SDDSFailure();

  std::string s(*static_cast<char **>(mem));
  EXPECT_STREQ("BPM02", s.c_str());
  delete t;
}

TEST(sdds, Column) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.twi")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetColumn(t, const_cast<char*>("s"));
  if (mem == NULL)
    throw sddsi::SDDSFailure();

  double *array = static_cast<double *>(mem);
  unsigned int length=SDDS_CountRowsOfInterest(t); 
  std::vector<double> x(array, array+length);

  // for (auto &val : x) {
  //   std::cout << val << std::endl;
  // }
  
  EXPECT_EQ(333u, x.size());
  EXPECT_DOUBLE_EQ(0.0, x[0]);
  EXPECT_NEAR(164.4008, x[332],0.0001);
  delete t;
}

TEST(sdds, ColumnValues) {
 SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.twi")) != 1 )
    throw sddsi::SDDSFailure();

  SDDS_ReadTable(t);
  void *mem;
  double s;
  std::vector<double> x;
  unsigned int length=SDDS_CountRowsOfInterest(t);
  for (auto i=0u; i<length; i++) {
    mem = SDDS_GetValue(t, const_cast<char*>("s"), i, NULL);
    if (mem == NULL) throw sddsi::SDDSFailure();
    s = *static_cast<double *>(mem);
    x.push_back(std::move(*static_cast<double *>(mem)));
  }
  
  EXPECT_NEAR(164.4008, s, 0.0001);
  EXPECT_EQ(333u, x.size());
  EXPECT_DOUBLE_EQ(0.0, x[0]);
  EXPECT_NEAR(164.4008, x[332],0.0001);
  delete t;
}

TEST(sdds, Pages) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice002.w")) != 1 )
    throw sddsi::SDDSFailure();

  // SDDS_ReadTable gives first page, has to be repeated for every page.
  // returns -1 if no more pages to read.

  int status = SDDS_ReadTable(t);
  unsigned int i=0;
  while (status != -1) {
    void *mem = SDDS_GetColumn(t, const_cast<char*>("p"));
    if (mem == NULL)
      throw sddsi::SDDSFailure();
    
    double *array = static_cast<double *>(mem);
    unsigned int length=SDDS_CountRowsOfInterest(t); 
    std::vector<double> x(array, array+length);
    EXPECT_EQ(100u, x.size());

    void *parmem = SDDS_GetParameter(t, const_cast<char*>("Pass"), NULL);
    if (parmem == NULL) throw sddsi::SDDSFailure();
    unsigned int turn = *static_cast<unsigned int *>(parmem);
    EXPECT_EQ(i, turn);

    i++;
    status = SDDS_ReadTable(t);
  }
  delete t;
}

TEST(sdds, FilterParticleId) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice002.w")) != 1 )
    throw sddsi::SDDSFailure();

  // filter single particle id
  // by SDDS_FilterRowsOfInterest AFTER(!) EACH(!) SDDS_ReadTable call
  // Default are all Rows => use Logic Macro SDDS_AND to get only the filtered Rows
  unsigned int id=2u;

  int status = SDDS_ReadTable(t);
  int ret = SDDS_FilterRowsOfInterest(t,const_cast<char*>("particleID"),id,id,SDDS_AND);
  if (ret == -1) throw sddsi::SDDSFailure();
  ASSERT_EQ(1,ret);

  unsigned int i=0;
  while (status != -1) {
    void *mem = SDDS_GetColumn(t, const_cast<char*>("p"));
    if (mem == NULL)
      throw sddsi::SDDSFailure();
    
    double *array = static_cast<double *>(mem);
    unsigned int length=SDDS_CountRowsOfInterest(t); 
    std::vector<double> x(array, array+length);
    EXPECT_EQ(1u, length);
    EXPECT_EQ(1u, x.size());

    void *parmem = SDDS_GetParameter(t, const_cast<char*>("Pass"), NULL);
    if (parmem == NULL) throw sddsi::SDDSFailure();
    unsigned int turn = *static_cast<unsigned int *>(parmem);
    EXPECT_EQ(i, turn);

    i++;
    status = SDDS_ReadTable(t);
    int ret = SDDS_FilterRowsOfInterest(t,const_cast<char*>("particleID"),id,id,SDDS_0_PREVIOUS);
    if (ret == -1) throw sddsi::SDDSFailure();
  }
  delete t;
}




// TEST(sddsFoP, OrbitTest) {
//   pal::SimToolInstance elegant(pal::elegant, pal::online, "elsa.lte");
//   elegant.SDDS = true;
  
//   pal::FunctionOfPos<pal::AccPair> orbit(elegant);
//   orbit.simToolClosedOrbit(elegant);
//   orbit.print("sdds-orbit.dat");
// }

TEST(sddsSimTool, Parameter) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
  elegant.set_sddsMode(true);

  double p = elegant.readParameter<double>(elegant.twiss(), "pCentral");
  EXPECT_NEAR(4.500987e+03, p, 0.001);
}

TEST(sddsSimTool, Table) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
  elegant.set_sddsMode(true);

  std::vector<std::string> columnKeys = {"s", "x", "y"};
  pal::SimToolTable tab = elegant.readTable(elegant.orbit(), columnKeys);

  EXPECT_EQ(333u, tab.rows());
  EXPECT_EQ(3u, tab.columns());
  EXPECT_NEAR(164.4008, tab.getd(332,"s"), 0.0001);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
