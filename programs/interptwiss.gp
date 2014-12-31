set grid
set xlabel "s / m"
set ylabel "beta_x / m"

plot 'twiss.dat' u 1:4 t 'data', 'twiss_interp.dat' u 1:2 w l t 'interpolation'
pause mouse close
