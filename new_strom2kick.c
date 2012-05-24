/* ------new_strom2kick------
 * ==============================================================
 * special version of strom2kick
 * only for "Spuren" from 2012-03-25 up to 2012-04-12
 * older "Spuren" have wrong CORRS.TIMING format, use strom2kick.
 * newer "Spuren" already have correct .KICK files
 * ==============================================================
 * Calculate vertical corrector-kickangles from currents saved with "Spuren"
 * This function was added to /sgt/ccs/apl/bpms/read_traces.cc in Jan. 2012
 * and updated to new corrector power-supplies on 25.03.2012 (CORRS.TIMING)
 * This version writes .KICK files compatible with new version of Bsupply
 * compile with "make" or "make new_strom2kick"
 * no argument: menu is called; argument spuren-path: direct run
 * 24.05.2012 - J.Schmidt
 */

#include <stdio.h>
#include <math.h>

#define NUMCORR 32       //number of vertical correctors
#define NUMTIMING 5000   //max. number of lines in timing- & strom-files
#define KF 0.62713       //default(!) magnet strengths. written to optics.dat for "Bsupply"
#define KD 0.57982
#define MF 3.5
#define MD 2.0


int run(const char *path, const int schleppfehler, const int t_rdownstart, const float rdownspeed);
int readfile(char *filename, float *array, int length);


int main(int argc, char *argv[])
{
  int choice;
  int loop = 1;
  char path[1024], folder[1024], wholepath[1024];
  
 //parameters (user input, default values(!))
  snprintf(path, 1024, "/home/jan/Kontrollsystem/");
  snprintf(folder, 1024, "2012-01-24-15-11-27");
  int schleppfehler = 0;     //ms
  int t_rdownstart = 4000;    //ms
  float rdownspeed = -0.004007;  //GeV/ms

  //direct run with path as argument
  if(argc==2) {
    run(argv[1], schleppfehler, t_rdownstart, rdownspeed);
    return 0;
  }

  //menu (if executed without argument)
  printf("\nATTENTION: Do not use with Spuren recorded before 2012-03-25 (new CORRS.TIMING format)\n\n");
  while(loop == 1)
    {
      printf("---------new_strom2kick---------\n");
      printf("(1) Pfad zu Spuren: %s\n", path);
      printf("(2) Spuren: %s\n", folder);
      printf("(3) Schleppfehler: %d ms\n", schleppfehler);
      printf("(4) Start Runter-Rampe: %d ms\n", t_rdownstart);
      printf("(5) Runter-Rampe: %.3f GeV/s\n", rdownspeed*1000);
      printf("(6) Run\n");
      printf("(0) Exit\n");
      printf("----------------------------\n");
      if (scanf("%d", &choice) != 1) {
	getchar(); choice=9;
      }
      
      switch(choice)
	{
	case 1:
	  printf("neuer Pfad zu Spuren: ");
	  scanf("%1024s", path);
	  break;
	case 2:
	  printf("neue Spuren: ");
	  scanf("%s1024", folder);
	  break;
	case 3:
	  printf("neuer Schleppfehler: ");
	  scanf("%d", &schleppfehler);
	  break;
	case 4:
	  printf("neuer Startzeitpunkt Runter-Rampe (ms): ");
	  scanf("%d", &t_rdownstart);
	  break;
	case 5:
	  printf("neue Geschwindigkeit Runter-Rampe (GeV/s): ");
	  scanf("%f", &rdownspeed);
	  rdownspeed /= 1000;   //GeV/s -> GeV/ms
	  break;
	case 6:
	  snprintf(wholepath, 1024, "%s%s", path, folder);
	  run(wholepath, schleppfehler, t_rdownstart, rdownspeed);
	  break;
	case 0:
	  loop = 0;
	  break;
	default:
	  printf("Keine gültige Eingabe.\n");
	  break;
	}
    }

  return 0;
}



int run(const char *path, const int schleppfehler, const int t_rdownstart, const float rdownspeed)
{
  unsigned int i, j, n=0;
  int num_strom, t_rupstart, t_rupstop;
  char filename[1024];
  float status[NUMCORR], timing[NUMTIMING];
  float strom[NUMTIMING], scaling[NUMCORR];
  float kick, energy, rupspeed, E_inj, E_ext, f_tmp;
  FILE *file, *out;

  printf("\nATTENTION: Do not use with Spuren recorded before 2012-03-25 (new CORRS.TIMING format)\n");
  printf("\nStart calculation:\n");

  //read E-ramp
  snprintf(filename, 1024, "%s/timing.dat", path);
  if ((file = fopen(filename, "r")) == NULL) {
    printf("ERROR: Cannot open %s.\n", filename);
    return 1;
  }
  fscanf(file, " ELS_ENERGY_MODEL.INJENERGY_AC %f\n", &E_inj);
  fscanf(file, " ELS_ENERGY_MODEL.EXTENERGY_AC %f\n", &E_ext);
  fscanf(file, " ELS_TIMING_CYCLE.RAMPSTART_AC %f\n", &f_tmp);
  t_rupstart = (int)floor(f_tmp*1000+0.5);                      //s -> ms
  fscanf(file, " ELS_TIMING_CYCLE.RAMPSTOP_AC %f\n", &f_tmp);
  t_rupstop = (int)floor(f_tmp*1000+0.5);                       //s -> ms
  fscanf(file, " ELS_ENERGY_MODEL.UPSLOPE_AC %f\n", &rupspeed);
  rupspeed /= 1000.0;                                         //GeV/s -> GeV/ms

  printf("Injektion %g GeV\tExtraktion %g GeV\n", E_inj, E_ext);
  printf("Rampstart %d ms\tRampstop %d ms\tRampe %g GeV/s\n", t_rupstart, t_rupstop, rupspeed*1000);


  //read status
  snprintf(filename, 1024, "%s/correctors/VCORRS.STATUS", path);
  n = readfile(filename, status, NUMCORR);
  printf("Read %d status values\t", n);
  //read scaling
  snprintf(filename, 1024, "%s/correctors/VCORRS.SCALING", path);
  n = readfile(filename, scaling, NUMCORR);
  printf("read %d scaling values\t", n);
  //read timing
  snprintf(filename, 1024, "%s/correctors/CORRS.TIMING", path);
  n = readfile(filename, timing, NUMTIMING);
  printf("read %d timing values\n", n);

  printf("Write kicks:\n");


  for(i=0; i<NUMCORR; i++) {
    if (status[i] == 0.0)
      continue;

    //read current
    snprintf(filename, 1024, "%s/correctors/VC%02d.STROM", path, i+1);
    num_strom = readfile(filename, strom, NUMTIMING);


    //calculate & write kick
    snprintf(filename, 1024, "%s/correctors/VC%02d.KICK", path, i+1);
    if ((out = fopen(filename, "w")) == NULL)
      {
	printf("ERROR: Cannot open %s.\n", filename);
	return -1;
      }
    
    fprintf(out, "#t[ms]  kick[mrad]\n");
    
    for (j=0; j<num_strom; j++)
      {
	    //get energy(t_cycle)
	    if (timing[j] >= t_rupstart && timing[j] < t_rupstop)
	      energy = E_inj + rupspeed * (timing[j] - t_rupstart);
	    else if (timing[j] >= t_rupstop && timing[j] < t_rdownstart)
	      energy = E_ext;
	    else if (timing[j] >= t_rdownstart && energy > E_inj)
	      energy = E_ext + rdownspeed * (timing[j] - t_rdownstart);
	    else
	      energy = E_inj;
	    
	    kick = strom[j] / scaling[i] / energy;
	    fprintf(out, "%0.lf     %g\n", timing[j]+schleppfehler, kick);
      }
    printf("VC%02d(%d) ", i+1, num_strom);
    if ((i+1)%10 == 0)
      printf("\n");
    fclose(out);
  }
  printf("\n");


  //write magnet-strengths (fixed values, no input)
  snprintf(filename, 1024, "%s/optics.dat", path);
  if ((out = fopen(filename, "w")) == NULL)
    {
      printf("ERROR: Cannot open %s.\n", filename);
      return -1;
    }
  fprintf(out, "# these are default values,\n");
  fprintf(out, "# exact values while measurement are unknown\n\n");
  fprintf(out, "ELS_MAGNETE_QUADF.KF_AC   %g\n", KF);
  fprintf(out, "ELS_MAGNETE_QUADD.KD_AC   %g\n", KD);
  fprintf(out, "ELS_MAGNETE_SEXTF.MF_AC   %g\n", MF);
  fprintf(out, "ELS_MAGNETE_SEXTD.MD_AC   %g\n", MD);
  fclose(out);


  printf("Finished.\n\n");

  return 0;
}



//read file -> array, return number of read values
int readfile(char *filename, float *array, int length)
{
  int tmp, n=0;
  FILE *file;

  if ((file = fopen(filename, "r")) == NULL) {
    printf("ERROR: Cannot open %s.\n", filename);
    return -1;
  }

  while(fscanf(file, "%d %f\n", &tmp, &array[n]) != EOF) {
    n++;
    if (n>=length) break;
  }

  return n;
}
