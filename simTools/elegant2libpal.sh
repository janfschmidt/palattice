#!/bin/bash
# generate ascii files in a format readable by libpal
# from elegant output, via sdds-tools
# argument 1: tag for filename. if tag is "none", no tag is used 
# argument 2: filename (the "%s" part in elegant)
# argument 3: particleID, which trajectory is exported (irrelevant for closed orbit and lattice)

echo $1
if [ "$1" == "none" ];
then
    tag=""
else
    tag="_"$1
fi


# ascii closed orbit file elegant.clo
sddsprintout $2.clo elegant$tag.clo -Title='***' -col='(ElementName,ElementType,s,x,y)'

# header data for lattice parameter file elegant.param:
# circumference from watch file
file=`ls $2*.w | head -n1`
echo -e "circumference\t `sdds2stream $file -page=1 -par=PassLength`" > elegant$tag.param #overwrite existing file
# pCentral from watch file
echo -e "pCentral/m_e*c\t `sdds2stream $file -page=1 -par=pCentral`" >> elegant$tag.param #append..
# tunes from watch file
echo -e "tune:Qx\t `sdds2stream $2.twi -par=nux`" >> elegant$tag.param
echo -e "tune:Qz\t `sdds2stream $2.twi -par=nuy`" >> elegant$tag.param

# ascii lattice parameter file elegant.param
sddsprintout $2.param tmp.param -Title='***' -col='(ElementName,ElementParameter,ParameterValue,ElementType)'
cat tmp.param >> elegant$tag.param
rm tmp.param

# ascii twiss file elegant.twi
sddsprintout $2.twi elegant$tag.twi -Title='***' -width=200 -col='(ElementName,ElementType,s,betax,alphax,psix,etax,etaxp,betay,alphay,psiy)'

# ascii single particle trajectory files ($3=particleID) , e.g. elegant.w02.p1
# from watch-files:
numfiles=`ls $2*.w | wc -w`
for (( k=1; k<=$numfiles; k++ ))
do
    # filenames
    kk=`echo "$k-1" | bc`
    file_in=`ls $2*.w | awk -v k=$k NR==k{print}`
    file_out=`printf "elegant$tag.w%04i.p%i" $kk $3`
    # position s (as header)
    echo -e "position_s/m\t `sdds2stream $file_in -page=1 -par=s`" > $file_out
    # trajectory data
    sddsprocess $file_in -pipe=out -filter,column,particleID,$3,$3 -define,column,Pass,"Pass 1 +",type=long | sddscombine -pipe -merge | sddsprintout -pipe=in -Title='***' -col='(x,xp,y,yp,t,p,particleID,Pass)' >> $file_out
done
