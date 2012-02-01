# plots of polsim-output data
# all data for a specific moment of elsa-cycle (input of polsim)

set terminal postscript eps enhanced color "Arial" 16
set key
set grid

# 1.
# uses elsabpms.dat & orbit.dat (output from polsim)
# to plot x and z orbit

set out "output/orbit.eps"
set title "ELSA Orbit"
set xlabel "Position / m"
set ylabel "Ablage / mm"

plot "output/elsabpms.dat" u 1:2 w lines title "BPMs x", "output/elsabpms.dat" u 1:3 w lines title "BPMs z", "output/orbit.dat" u 1:2 w lines title "interpolated x", "output/orbit.dat" u 1:3 w lines title "interpolated z"


# 2.
# uses fields.dat & eval.dat (output from polsim)
# to plot B_x and B_z and its cos-evaluation

set xlabel "s / m"
set ylabel "B / 1/m"

set out "output/fields_z.eps"
set title "ELSA B_{z}"
plot "output/fields.dat" u 1:4 w impulses title "magnet data", "output/eval.dat" u 1:3 w lines title "cos evaluation"

set out "output/fields_x.eps"
set title "ELSA B_{x}"
plot "output/fields.dat" u 1:3 w impulses title "magnet data", "output/eval.dat" u 1:2 w lines title "cos evaluation"


# 3.
# uses horizontal.spectrum & vertical.spectrum (output from polsim)
# to plot Amplitude Spectra of B_x and B_z

unset key
set grid noxtics
set xtics 10
set mxtics 10
set xrange [-0.5:*]
set xlabel "Frequency / rev. harmonics"
set ylabel "Amplitude / 1/m"

set out "output/spectrum_z.eps"
set title "ELSA B_{z} Spectrum"
plot "output/vertical.spectrum" u 2 w impulses

set out "output/spectrum_x.eps"
set title "ELSA B_{x} Spectrum"
plot "output/horizontal.spectrum" u 2 w impulses


