#!/bin/bash

#path to plugin (requires that .so has been built)
PLUGIN_PATH="../build/libPchorAnalyzerPlugin.so"

TEST_ROOT="./"

for dir in "$TEST_ROOT"/*/; do
    #check if desc.txt exists in folder
    txtfile=$(find "$dir" -maxdepth 1 -name "desc.txt" )
    if [[ -n "$txtfile" ]]; then
        echo "=============================="
        echo "Test description for $dir:"
        echo "------------------------------"
        cat "$txtfile"
        echo "------------------------------"

        #we try all combinations of .cpp and .cor
        for cpp in "$dir"*.cpp; do

            for cor in "$dir"*.cor; do
                echo "Running Pchor on: "
                echo "  C++: $cpp"
                echo "  COR: $cor \n"
                echo "------------------------------"

                clang++-18 -std=c++23 -Xclang -load -Xclang "$PLUGIN_PATH" \
                    -Xclang -plugin-arg-PchorAnalyzer -Xclang --cor="$cor" \
                    "$cpp" -o /dev/null
                echo "------------------------------"
            done
        done
    fi
done