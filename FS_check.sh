#!/bin/bash

# *-------------------- `check.sh` Exit Codes --------------------* #
#  0 : Success              :   Student gets at least 50%
#  1 : Student error        :   `diff` fail, timeouts, etc
#  2 : Development error    :   Missing testcase dirs, missing output files, etc

# Files
makefile=Makefile
required_source_files=("assg.c" "spheres.c" "vector.c" "color.c")  # No color.c for MS1
executables=("MS1_assg" "MS2_assg")
program_name=FS_assg

# Compilation and linking flags and time limit
compilation_flags="-Wall -Werror -std=c99 -DFS"
linking_flags="-lm"
time_limit=2

# Required directories
source_dir="./src"
bin_dir="./bin"
testcases_dir="./FS_Testcases"

# Postfixes for files found or generated in the testcases directory
input_postfix="_input.txt"
output_postfix="_output.ppm"
student_postfix="_student.ppm"

# Logging functions and starting message
width=$(tput cols);
log()     { printf "\033[00m[CHECK] %s\033[0m\n" "$1"; }
info()    { printf "\033[33m[CHECK] %s\033[0m\n" "$1"; }
success() { printf "\033[32m[CHECK] %s\033[0m\n" "$1"; }
warning() { printf "\033[35m[CHECK] Warning: %s\033[0m\n" "$1"; }
br()      { break=$(printf "%*s" "$((width-8))" "" | tr " " "-"); log "$break"; }
error() {
    printf "\033[31m[CHECK] Error: %s\033[0m\n" "$1";
    log ""
    br; info "END: Check failed."; br; echo;
    exit "$2";
}
doesPass=1
errorCont() {
    doesPass=0
    printf "\033[31m[CHECK] Error: %s\033[0m\n" "$1";

    # Get user input
    printf "\033[33m[CHECK] Enter to continue, or type anything to exit: \033[0m"
    read input

    # Terminate
    if [[ -n "$input" ]]; then
        info "Exiting with errors..."
        log ""
        br; info "END: Check failed."; br; echo;
        exit "$2";
    fi

    info "Continuing..."
    
}
echo; br; success "START"; br; log ""

# Check if required directories exists
if [ ! -d "$testcases_dir" ]; then
    error "Expected test cases directory '$testcases_dir' does not exist." 2
fi
if [ ! -d "$source_dir" ]; then
    error "Expected source directory '$source_dir' does not exist." 1
fi

# Check that all required files exist
for file in "${required_source_files[@]}"; do
    if [ ! -e "$source_dir/$file" ]; then
        error "Required file '$source_dir/$file' not found." 1
    fi
done
if [ ! -e "$makefile" ]; then
    error "$makefile does not exist." 1
fi

# Remove executables if they exist
rm -f "$program_name"
for exe in "${executables[@]}"; do
    rm -f "$exe"
done

# Compile and link using Makefile
log "Compiling and linking using Makefile..."
if ! make -f "$makefile" > /dev/null; then
    error "$makefile failed to run." 1
fi
exe_path="./$program_name"

# Check if the makefile created the executables
if [ ! -e "$program_name" ]; then
    error "$makefile failed to create $program_name." 1
fi
success "$makefile successfully created $program_name."
for exe in "${executables[@]}"; do
    if [ ! -e "$exe" ]; then
        errorCont "$makefile failed to create $exe." 1
    else
        success "$makefile successfully created $exe."
    fi
done

log "Checking '$program_name'..."

# Iterate over every input file corresponding to the exercise number
for input_path in "$testcases_dir"/1[0-9]"$input_postfix"; do

    log ""

    # Check if the corresponding output file exists. If it doesn't then skip this test case
    output_path=${input_path/"$input_postfix"/"$output_postfix"}
    if [[ ! -e $output_path ]]; then
        info "Missing output file: '$output_path'. Skipping test case."
        continue
    fi

    # Set timeout and generate student output file
    student_output_path=${input_path/"$input_postfix"/"$student_postfix"}
    log "Testing using '$input_path'..."
    timeout "$time_limit" "$exe_path" "$input_path" "$student_output_path"
    
    # Check if program took longer than the time limit, or the program exited with non-zero code
    exit_code=$?
    if [ $exit_code -eq 124 ]; then
        errorCont "'$program_name' timed out." 1
        continue
    elif [ $exit_code -ne 0 ]; then
        errorCont "'$program_name' exited with non-zero code." 1
        continue
    else
        success "'$program_name' terminated successfully under the time limit."        
    fi

    # Check for memory leaks
    log "Checking for memory leaks using Valgrind..."
    valgrind_output=$(valgrind "$exe_path" "$input_path" "$student_output_path" 2>&1)
    bytes=$(echo "$valgrind_output" | grep "in use at exit" | awk '{print $6}')
    blocks=$(echo "$valgrind_output" | grep "in use at exit" | awk '{print $9}')

    if [ "$bytes" -ne 0 ] || [ "$blocks" -ne 0 ]; then
        errorCont "Memory leaks found in '$program_name': in use at exit: $bytes bytes in $blocks blocks. You will not get at least 50% in this milestone." 1
    else
        success "'$program_name' terminated with no memory leaks."        
    fi

    # Run `ppmcmp` then check if the output is less than or equal to 5
    ppmcmp=$(python3 ppmcmp.py "$output_path" "$student_output_path" 2> /dev/null)
    exit_code=$?

    if [ $exit_code -eq 2 ]; then
        errorCont "Invalid PPM format." 1
    elif [ $exit_code -eq 3 ]; then
        errorCont "Max color value should be 255." 1
    elif [ $exit_code -eq 4 ]; then
        errorCont "Incorrect image dimensions." 1
    elif [ $exit_code -eq 5 ]; then
        errorCont "Encountered value out of the range [0, 255]." 1
    elif [ $exit_code -ne 0 ]; then
        errorCont "ppmcmp.py terminated with errors." 1
    elif awk "BEGIN {exit !($ppmcmp <= 5)}"; then
        success "Your program's output for "$input_path":"
        ./viewppm "$student_output_path"
        success "'$student_output_path' is close enough to the output: ppmcmp = $ppmcmp"
    else
        info "Expected output for "$input_path":"
        ./viewppm "$output_path"
        info "Your program's output for "$input_path":"
        ./viewppm "$student_output_path"
        errorCont "'$program_name' does not give expected output: ppmcmp = $ppmcmp. Expected image and your program's image are shown above." 1
    fi
    
done

# If check failed
if [ "$doesPass" -eq 0 ]; then
    log ""
    error "Reached end with errors" 1
fi

# Ending message
log ""; br; success "END: Passed! You will get at least 50% on the final submission portion of this assignment."; br; echo;
