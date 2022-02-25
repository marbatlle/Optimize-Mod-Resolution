#!/bin/bash
set -euo pipefail

printf '\n************************************************************************' 
printf '\n*** DETERMINING AN OPTIMAL MODULARITY RESOLUTION PARAMETER FOR MOLTI ***' 
printf '\n************************************************************************\n\n' 

# Defining default values for parameters
lower_limit=0
upper_limit=50
steps=5
randomizations=5
min_nodes=6

# Classifying input arguments
while getopts "l:u:s:r:m:" opt; do
  case $opt in
    l) lower_limit=($OPTARG)                ;;
    u) upper_limit=($OPTARG)                ;;
    s) steps=($OPTARG)                      ;;
    r) randomizations=($OPTARG)             ;;
    m) min_nodes=($OPTARG)
  esac
done

# STEP 0
printf '0/2 - Check inputs and prepare environment\n' 

## Check if variables are integers
re='^[0-9]+$'
if ! [[ $lower_limit =~ $re ]] || ! [[ $upper_limit =~ $re ]] || ! [[ $steps =~ $re ]] || ! [[ $randomizations =~ $re ]] || ! [[ $randomizations =~ $re ]]; then
   echo "   error: All variables should be an integer numbers" >&2; exit 1
fi

## Check if lower limit is larger than upper limit
if [[ $lower_limit > $upper_limit ]] ; then
   echo "   error: The lower limit should be smaller than the upper limit" >&2; exit 1
fi

## Checking input format
if ! ls ./input/networks/*.csv > /dev/null; then
    echo "  error: Network layers should follow the 'csv' format"
    exit
fi

rm -r -f output ; mkdir output; rm -r -f src/networks
mkdir -p src/networks; cp -a input/networks/*.csv src/networks
for file in src/networks/*.csv; do 
    sed -i 's/,/ /g' $file ; mv "$file" "${file/.*/.gr}"
done

# STEP 1
printf '1/2 - Defining community structure for each modularity\n'
printf "      Lower Resolution Limit: %s\n      Upper Resolution Limit: %s\n      Steps size: %s\n" $lower_limit $upper_limit $steps

mkdir output/clusters
networks=$(ls src/networks/*.gr)
networks=$(ls src/networks/*.gr)

echo -n '      Process: '

chmod 777 src/MolTi-DREAM/src/molti-console
src/MolTi-DREAM/src/molti-console -r $randomizations -p 0 -o output/clusters/communities00 ${networks} >& /dev/null

if ! ls output/clusters/communities00_effectif.csv > /dev/null; then
    echo "     error: Error when running MolTi-DREAM"
    rm -r -f output; rm -r -f src/networks
    exit
fi

DOT=.
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    echo -n $DOT
    src/MolTi-DREAM/src/molti-console -r $randomizations -p ${COUNTER} -o output/clusters/communities${COUNTER} ${networks} >& /dev/null
done

# Filtering small communities
awk -v var=$min_nodes ' { if($2 > var) print; } ' output/clusters/communities00_effectif.csv > output/clusters/communities00_effectif.tmp 
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
awk -v var=$min_nodes ' { if($2 > var) print; } ' output/clusters/communities${COUNTER}_effectif.csv > output/clusters/communities${COUNTER}_effectif.tmp
done

# STEP 2
printf '\n2/2 - Obtaining optimal modularity value\n'

# Procesing number of randomizations
echo '00' >> output/output_mod_parameter.txt
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    echo $COUNTER
done >> output/output_mod_parameter.txt

# Procesing number of communities
cat output/clusters/communities00_effectif.tmp | cut -d":" -f2 | cut -f 1 | wc -l >> output/output_num_communities.txt
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    cat output/clusters/communities${COUNTER}_effectif.tmp | cut -d":" -f2 | cut -f 1 | wc -l
done >> output/output_num_communities.txt

# Procesing average size of communities
cat output/clusters/communities00_effectif.tmp | cut -d":" -f2 | cut -f 2 >> output/tmp_avg_com_size.txt; awk '{ total += $1; count++ } END { print total/count }' output/tmp_avg_com_size.txt | tr , . >> output/output_avg_com_size.txt; rm -f output/tmp_avg_com_size.txt
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    cat output/clusters/communities${COUNTER}_effectif.tmp | cut -d":" -f2 | cut -f 2 >> output/tmp_avg_com_size.txt
    awk '{ total += $1; count++ } END { print total/count }' output/tmp_avg_com_size.txt | tr , .
    rm -f output/tmp_avg_com_size.txt
done >> output/output_avg_com_size.txt

# Determining optimal mod
python src/optimize_mod_parameter.py > output/optimal_mod_parameter.txt #2>/dev/null
if ! ls output/optimal_mod_parameter.txt > /dev/null; then
    echo "     error: Could not define an optimal value for this community structure"
    rm -r -f src/networks
    rm -f output/output_*.txt
    exit
fi

# clean output directory
rm -f output/clusters/communities00*; rm -f output/clusters/*.tmp; rm -r -f src/networks; rm -f output/output_*.txt
cat output/optimal_mod_parameter.txt
touch output/.gitkeep


