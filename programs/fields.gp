set grid
set xlabel "s / m"
set ylabel "B / 1/m"

plot 0 w l lc 0 notitle, 'fields_noedge.dat' u 1:4 w l lc 1 title "noedge",\
     'fields_edge.dat' u 1:4 w l lc 2 title "edge"

# plot 'fields_noedge.dat' u 1:5 w l title "noedge",\
#      'fields_edge.dat' u 1:5 w l title "edge"

pause mouse close
