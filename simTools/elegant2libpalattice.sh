#!/bin/bash
# generate ascii files in a format readable by libpalattice
# from elegant output, via sdds-tools
# argument 1: tag for filename. if tag is "none", no tag is used 
# argument 2: filename (the "%s" part in elegant)
# argument 3: switch for watch file ascii export. if $3 is "watchfiles", export is done

if [ "$1" == "none" ];
then
    tag=""
else
    tag="_"$1
fi

# header data for twiss file elegant.twi from .twi file:
# circumference (position s of last element)
circ=`sdds2stream $2.twi -col=s | tail -n1`
echo -e "circumference\t $circ" > elegant$tag.twi #overwrite existing file
# pCentral
echo -e "pCentral `sdds2stream $2.twi -par=pCentral`" >> elegant$tag.twi #append..
# tunes 
echo -e "nux\t `sdds2stream $2.twi -par=nux`" >> elegant$tag.twi
echo -e "nuy\t `sdds2stream $2.twi -par=nuy`" >> elegant$tag.twi
# momentum compaction factor
echo -e "alphac\t `sdds2stream $2.twi -par=alphac`" >> elegant$tag.twi

# ascii closed orbit file elegant.clo
sddsprocess $2.clo -pipe=out -match=column,ElementType=WATCH,! -match=column,ElementName=_BEG_,! | sddsprintout -pipe=in elegant$tag.clo -Title='***' -col='(ElementName,ElementType,s,x,y)'

# ascii lattice parameter file elegant.param
sddsprintout $2.param elegant$tag.param -Title='***' -col=ElementName -col=ElementParameter -col=ParameterValue,format=%.12e -col=ElementType

# ascii twiss file elegant.twi
sddsprocess $2.twi -pipe=out -match=column,ElementType=WATCH,! -match=column,ElementName=_BEG_,! | sddsprintout -pipe=in tmp.twi -Title='***' -width=200 -col='(ElementName,ElementType,s,betax,alphax,psix,etax,etaxp,betay,alphay,psiy)'
cat tmp.twi >> elegant$tag.twi
rm tmp.twi

# ascii single particle trajectory files for all particles, e.g. elegant.w02.p1
# from watch-files:
if [ "$3" == "watchfiles" ]; then
    numParticles=`sdds2stream ${2}000.w -par=Particles -page=1`
    numfiles=`ls $2*.w | wc -w`
    echo "write $numfiles watch files for each of $numParticles particles"
    for (( pID=1; pID<=$numParticles; pID++ )); do
	for (( k=1; k<=$numfiles; k++ )); do
	    # filenames
	    kk=`echo "$k-1" | bc`
	    file_in=`ls $2*.w | awk -v k=$k NR==k{print}`
	    file_out=`printf "elegant$tag.w%04i.p%i" $kk $pID`
	    # position s (as header)
	    echo -e "position_s/m\t `sdds2stream $file_in -page=1 -par=s`" > $file_out
	    # trajectory data
	    sddsprocess $file_in -pipe=out -filter,column,particleID,$pID,$pID -define,column,Turn,"Pass 1 +",type=long | sddscombine -pipe -merge | sddsprintout -pipe=in -Title='***' -col='(x,xp,y,yp,t,p,particleID,Turn)' >> $file_out
	done
	echo "$numfiles watch files for particle $pID written"
    done
fi
