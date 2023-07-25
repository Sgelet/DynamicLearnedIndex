mapfile -t files <<< "$(ls -1 *.data)"

declare -a variations=(
  "CH"
  "CQ"
  "GA"
  "ED"
)

variant=0
length=${#files[@]}

for i in "${variations[@]}"
do
  ((variant++))
  for f in "${!files[@]}";
  do
   full="${files[$f]##*/}"
   filename="${full%.*}"
   echo "${files[$f]}"
   echo "$i" >> out_raw/$i${filename}.out
   cat "${files[$f]}" | ./cmake-build-release/DynamicConvexHull RUN $variant 10000 >> out_raw/$i${filename}.out
   cat "${files[$f]}" | ./cmake-build-release/DynamicConvexHull RUN $variant 10000 >> out_raw/$i${filename}.out
   cat "${files[$f]}" | ./cmake-build-release/DynamicConvexHull RUN $variant 10000 >> out_raw/$i${filename}.out
  done
done