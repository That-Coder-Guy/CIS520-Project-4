#!/bin/bash

# Define directories and output file
RESULTS_DIR="test_results"
OUTPUT_CSV="benchmark_summary.csv"

# Print the CSV header
echo "Cores,Memory_Per_Core,Wall_Time,User_Time,System_Time,CPU_Percent,Max_RSS_KB" > "$OUTPUT_CSV"

echo "Parsing result files..."

# Loop through all result files
for file in "$RESULTS_DIR"/results_*.txt; do
    # Skip if no files are found
    [ -e "$file" ] || continue

    # 1. Extract config values from the custom header in your slurm script
    cores=$(grep "TEST CONFIG:" "$file" | awk '{print $3}')
    mem=$(grep "TEST CONFIG:" "$file" | awk '{print $6}')

    # 2. Extract metrics from the /usr/bin/time -v output
    wall_time=$(grep "Elapsed (wall clock) time" "$file" | awk -F': ' '{print $2}')
    user_time=$(grep "User time (seconds)" "$file" | awk -F': ' '{print $2}')
    sys_time=$(grep "System time (seconds)" "$file" | awk -F': ' '{print $2}')
    cpu_percent=$(grep "Percent of CPU this job got" "$file" | awk -F': ' '{print $2}' | tr -d '%')
    max_rss=$(grep "Maximum resident set size" "$file" | awk -F': ' '{print $2}')

    # 3. Append the extracted data as a new row in the CSV
    echo "${cores},${mem},${wall_time},${user_time},${sys_time},${cpu_percent},${max_rss}" >> "$OUTPUT_CSV"
done

# Optional: Sort the CSV numerically by Cores (Column 1), keeping the header at the top
(head -n 1 "$OUTPUT_CSV" && tail -n +2 "$OUTPUT_CSV" | sort -t, -k1,1n) > "${OUTPUT_CSV}.tmp" && mv "${OUTPUT_CSV}.tmp" "$OUTPUT_CSV"

echo "Aggregation complete! Metrics saved to $OUTPUT_CSV"