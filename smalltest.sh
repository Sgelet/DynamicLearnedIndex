mapfile -t files <<< "$(ls -1 q*18.data)"

tag="qCH"
length=${#files[@]}
for f in "${!files[@]}";
do
  full="${files[$f]##*/}"
  filename="${full%.*}"
  echo "${files[$f]}"
  echo "${tag}" >> out_raw/${tag}${filename}.out
  cat "${files[$f]}" | ./cmake-build-release/DynamicConvexHull RUN 1000 >> out_raw/${tag}${filename}.out
  cat "${files[$f]}" | ./cmake-build-release/DynamicConvexHull RUN 1000 >> out_raw/${tag}${filename}.out
  cat "${files[$f]}" | ./cmake-build-release/DynamicConvexHull RUN 1000 >> out_raw/${tag}${filename}.out
done