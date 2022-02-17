#!/bin/bash
set -euo pipefail

# Defining default values for parameters
lower_limit=0
upper_limit=50
steps=5

# Classifying input arguments
while getopts "l:u:s:" opt; do
  case $opt in
    l) lower_limit=($OPTARG)                ;;
    u) upper_limit=($OPTARG)                ;;
    s) steps=($OPTARG)   
  esac
done

# STEP 0
echo '0/3 - Preparing environment' 
rm -r -f output ; mkdir output; rm -r -f src/networks
mkdir -p src/networks; cp -a input/networks/. src/networks
for file in src/networks/*; do sed -i 's/,/ /g' $file ; mv "$file" "${file/.*/.gr}"; done

# STEP 1
echo '1/3 - Defining community structure for each modularity'
printf "Lower Resolution Limit: %s\nUpper Resolution Limit: %s\nSteps size: %s\n" $lower_limit $upper_limit $steps

mkdir output/clusters
networks=$(ls src/networks/*.gr)
echo 'Processing base randomizations'
networks=$(ls src/networks/*.gr)
src/MolTi-DREAM-master/src/molti-console -r 5 -p 0 -o output/clusters/communities00 ${networks} >& /dev/null
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    echo Processing ${COUNTER} randomizations
    src/MolTi-DREAM-master/src/molti-console -r 5 -p ${COUNTER} -o output/clusters/communities${COUNTER} ${networks} >& /dev/null
done

# Filtering small communities
awk ' { if($2 > 6) print; } ' output/clusters/communities00_effectif.csv > output/clusters/communities00_effectif.tmp 
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
awk ' { if($2 > 6) print; } ' output/clusters/communities${COUNTER}_effectif.csv > output/clusters/communities${COUNTER}_effectif.tmp
done

# STEP 2
echo '2/3 - Analyzing comms'
# Procesing number of randomizations
echo '00' >> output/output_mod_parameter.txt
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    echo $COUNTER
done >> output/output_mod_parameter.txt
# Procesing number of communities
cat output/clusters/communities00_effectif.csv | cut -d":" -f2 | cut -f 1 | wc -l >> output/output_num_communities.txt
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    cat output/clusters/communities${COUNTER}_effectif.csv | cut -d":" -f2 | cut -f 1 | wc -l
done >> output/output_num_communities.txt
# Procesing average size of communities
cat output/clusters/communities00_effectif.csv | cut -d":" -f2 | cut -f 2 >> output/tmp_avg_com_size.txt; awk '{ total += $1; count++ } END { print total/count }' output/tmp_avg_com_size.txt | tr , . >> output/output_avg_com_size.txt; rm -f output/tmp_avg_com_size.txt
for (( COUNTER=$lower_limit; COUNTER<=$upper_limit; COUNTER+=$steps )); do
    cat output/clusters/communities${COUNTER}_effectif.csv | cut -d":" -f2 | cut -f 2 >> output/tmp_avg_com_size.txt
    awk '{ total += $1; count++ } END { print total/count }' output/tmp_avg_com_size.txt | tr , .
    rm -f output/tmp_avg_com_size.txt

done >> output/output_avg_com_size.txt
# STEP 3
echo '3/3 - Obtaining optimal randomizations value'
python scripts/optimize_mod_parameter.py #>> output/optimal_mod_parameter.txt 2>/dev/null

# clean output directory
rm -f output/clusters/communities00*; rm -f output/clusters/*.tmp; rm -r -f src/networks; rm -f output/*.txt

echo 'Process COMPLETED!'


