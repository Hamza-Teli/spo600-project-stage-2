/* This pass was creatd by Hamza Teli with the help of
Professor Chris Tyler's SPO600 wiki and lecture. 

This pass accomplishes the following:


*/
// These headers were taken from Professor Chris Tyler's Week 7 Lecture
// These headers were taken from Professor Chris Tyler's Week 7 Lecture
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "ssa.h"
#include "tree-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "internal-fn.h"
#include "gimple-pretty-print.h"
#include "cgraph.h"

// Added headers
#include "gimple-ssa.h"
#include "attribs.h"
#include "pretty-print.h"
#include "tree-inline.h"
#include "intl.h"
#include "function.h"
#include "basic-block.h"
#include <string>
#include <map>
#include <vector>
#include <cstdio>


// Namespace <--- This section I learned from SPO600 Week 7 - Class 1 Lecture from Professor Chris Tyler
namespace{
    const pass_data pass_data_hteli1 = {
            GIMPLE_PASS, /* type */
            "hteli1", /* name of my pass [We will use this inside passes.def as pass_hteli1_pass]*/ 
            OPTGROUP_NONE, /* optinfo_ flags */
            TV_NONE, /* tv_id */
            PROP_cfg, /* specify that we need properties */
            0, /* the properties provided */
            0, /* the properties destroyed */
            0, /* todo_flags_start */
            0, /* todo_flags_finish */
    };

    /*
        Please refer to the instructions below from Professor CHris Tyler that helped me build the class:
        Pass Code
            Each pass provides a C++ source file which provides:
            -   A pass_data structure which defines the pass details (defined in tree-pass.h);
            -   A class which defines a gimple_opt_pass and includes the public methods gate and execute.
            -   The gate method controls whether the pass is active. It can consider multiple factors, including command-line arguments, source language, and target. This method returns a boolean.
            -   The execute method executes the pass. It accepts a pointer to a function object and will be called once for each function in the unit being compiled (which is not always what you want!).
            -   A method in the anon namespace named make_pass_name which returns a pointer to the gimple_opt_pass described above.
    */


    // This is where you identify the class
    class pass_hteli1 : public gimple_opt_pass {
        public:

            // Constructor -------------------
            pass_hteli1(gcc::context *ctxt) : gimple_opt_pass(pass_data_hteli1, ctxt) {

            }

            // The gate function 
            bool gate(function *) final override {
                // return 
                return true;

            }

            // The execute function: this is where the magic happens
            unsigned int execute (function * /*func*/) override {
                // I created a static flag to ensure it only prints once
                static bool end = false;
                if (end) {
                    return 0;
                }

                // Lets create a map that holds the functions
                std::map<std::string, std::string> resolverMap;

                // another map to store the variant functions (the key is the basename and corresponding values are variants)
                std::map<std::string, std::vector<std::string>> variantMap;
                
                // Use cgraph node
                cgraph_node *node;

                // This is where we will get started with identifying the functions that have been cloned
                if (dump_file) {

                    // Lets use FOR_EACH_FUNCTION
                    FOR_EACH_FUNCTION(node) {
                        // Get the function pointer
                        function *current_function_pointer = node->get_fun();

                        // Validate
                        if (!current_function_pointer)
                            continue;
                        // Get the complete funciton name
                        std::string functionName(function_name(current_function_pointer));
                        
                        std::string baseName;
                        std::string suffix;

                        // Get the dot
                        size_t dot = functionName.find('.');

                        // Check if the dot is there
                        if (dot== std::string::npos){
                            // If no dot is found, treat it as default
                            baseName = functionName;
                            suffix = "default";
                        } 
                        else {
                            baseName = functionName.substr(0, dot);
                            suffix = functionName.substr(dot + 1);
                        }
                        
                        // Get the suffix first
                        //std::string suffix = functionName.substr(dot + 1);

                        // Now we check that if the function has a resolver suffix, we simply store its base name
                        if (suffix == "resolver") {
                          //std::string baseName = functionName.substr(0, dot);
                          resolverMap[baseName] = functionName;

                          // Show an output
                          fprintf(dump_file, "--------------------------------------------------------------------------------\n");
                          fprintf(dump_file, "**** ---> Resolver was found for base function: %s\n", baseName.c_str());
                        }
                    }

                    // Second pass goes here where we use the names inside our map and find all the variants
                    FOR_EACH_FUNCTION(node) {
                        // Get the function pointer
                        function *current_function_pointer = node->get_fun();

                        // Validate
                        if (!current_function_pointer)
                            continue;
                        // Get the complete funciton name
                        std::string functionName(function_name(current_function_pointer));

                        // Instantiate the variables
                        std::string baseName;
                        std::string suffix;

                        // Get the dot
                        size_t dot = functionName.find('.');

                        // Check if the dot is there
                        if (dot== std::string::npos){
                            // If there is no dot then we treat it as a default one
                            baseName = functionName;
                            suffix = "default";
                            functionName = baseName + ".default";
                        } 
                        else {
                            baseName = functionName.substr(0, dot);
                            suffix = functionName.substr(dot + 1);
                        }
                        

                        // Now we check that if the function has a resolver suffix, if so just continue
                        if (suffix == "resolver") {  
                            continue;
                        }
                
                        // Now we check if base has a resolver
                        if (resolverMap.find(baseName) != resolverMap.end()) {
                            variantMap[baseName].push_back(functionName);
                            fprintf(dump_file, "**** ---> Clone variant successfully found: %s (base function: %s) with resolver: %s\n", functionName.c_str(), baseName.c_str(), resolverMap[baseName].c_str());
                            fprintf(dump_file, "--------------------------------------------------------------------------------\n");
                        }
                    }

                    
                }

                // Custom function that prints the map created
                print_all_cloned_variants(variantMap, resolverMap);
                
                // Set end to true
                end = true;

                // Compare the functions
                compare_the_cloned_functions(variantMap);

                // Return value
                return 0;
            }
            
            // This function will take the resolver map and corresponding variants and print them
            void print_all_cloned_variants(const std::map<std::string, std::vector<std::string>> &variantMap, const std::map<std::string, std::string> &resolverMap) {

                // Use for loop
                for (const auto &element : variantMap) {
                    // Get the key and value
                    const std::string &baseName = element.first;
                    const std::vector<std::string> &variants = element.second;

                    // Now we print the resolver in a nice clean matter
                    fprintf(dump_file, "\n------------------------- Summary --------------------------\n");
                    fprintf(dump_file, "Resolver Function: %s\n", resolverMap.at(baseName).c_str());

                    // Now simply print the variants for that resolver function
                    for (const auto &variant : variants) {
                        fprintf(dump_file, " -------->  Variant: %s\n", variant.c_str());
                    }
                    fprintf(dump_file, "-----------------------------------------------------------\n");

                }
            }

            // This function will compare the two variants for each base function.
            // If the two functions are identical then it will print PRUNE, if not then NOPRUNE
            void compare_the_cloned_functions(const std::map<std::string, std::vector<std::string>> &variantMap) {
                // Lets loop and see if we have two variants
                for (const auto &base : variantMap) {

                    // Get the key and values
                    const std::string &baseName = base.first;
                    const std::vector<std::string> &variantValues = base.second;

                    // We assume that each base function has two clones
                    if (variantValues.size()!=2) {
                        fprintf(dump_file, "The number of variants for %s are not %d", baseName.c_str(), 2);
                        continue;
                    }

                    // Instantiate function pointers
                    function *functionOne = nullptr;
                    function *functionTwo = nullptr;

                    // Same logic as before
                    cgraph_node *node;
                    // Loop through each function and return the function we need
                    FOR_EACH_FUNCTION(node) {
                        function *fun = node->get_fun();

                        // Validate it
                        if (!fun) {
                            continue;
                        }
                        std::string functionName(function_name(fun));
                        // 
                        if (functionName == variantValues[0] || (functionName + ".default") == variantValues[0]) {
                            functionOne = fun;
                        }
                        else if (functionName == variantValues[1] || (functionName + ".default") == variantValues[1]) {
                            functionTwo = fun;
                        }
                    }

                    // Validate if both are valid
                    if (!functionOne || !functionTwo) {
                        fprintf(dump_file, "ERROR! Function 1 and function 2 could not be retrieved. Check the logic!\n");
                    }
                    else 
                    {
                        fprintf(dump_file, "Successfully retrieved both function 1 and two, the addresses are as follows: %p,  %p \n", (void*)functionOne, (void*)functionTwo);
                    }

                    // Over here we can check if basic blocks are the same.
                    int basicBlockCountForFunctionOne = 0;
                    int basicBlockCountForFunctionTwo = 0;
                    basic_block bb;

                    // We can use that macro Professor Chris Tyler suggested in the project stage 1 wikie
                    FOR_EACH_BB_FN(bb, functionOne) {
                        basicBlockCountForFunctionOne++;
                    }

                    // Same thing here for function two
                    FOR_EACH_BB_FN(bb, functionTwo) {
                        basicBlockCountForFunctionTwo++;
                    }

                    // Function names
                    std::string functionOneName = function_name(functionOne);
                    std::string functionTwoName = function_name(functionTwo);

                    // Lets print a basic block count for each function section here
                    fprintf(dump_file, "\n---------------------------------------------------------\n");
                    fprintf(dump_file, "----------------- Basic Block Counts ---------------------\n");
                    fprintf(dump_file, "Function 1: (%s) Block Count %d\n", functionOneName, basicBlockCountForFunctionOne.c_str());
                    fprintf(dump_file, "Function 2: (%s) Block Count %d\n", functionTwoName, basicBlockCountForFunctionTwo.c_str());
                    fprintf(dump_file, "---------------------------------------------------------\n");

                    // Set a flag
                    bool identical = false;
                    // What im doing here is checking if the basic block count is the same. The logic is that if the number of basic block counts differ across functions
                    // that means its most likely not identical.
                    if (basicBlockCountForFunctionOne == basicBlockCountForFunctionTwo) {
                        identical = true;
                    }

                    // Now check if it passes the first check
                    if(identical) {

                        // Print that we passed the first check
                        fprintf(dump_file, "Passed Check #1 (The number of basic block counts are the same for each function)\n");
                        // Now we can compare the gimple statement counts in each of the basic blocks
                        // Found the macro ENTRY_BLOCK_PTR_FOR_FN here -> https://github.com/gcc-mirror/gcc/blob/master/gcc/basic-block.h
                        basic_block basicBlockForFunctionOne = ENTRY_BLOCK_PTR_FOR_FN(functionOne)->next_bb;
                        basic_block basicBlockForFunctionTwo = ENTRY_BLOCK_PTR_FOR_FN(functionTwo)->next_bb;

                        while (basicBlockForFunctionOne && basicBlockForFunctionTwo && identical) {
                            // We can now count for the gimple statements
                            int gimpleCountForFunctionOne = 0;
                            int gimpleCountForFunctionTwo = 0;
                            for (gimple_stmt_iterator gsi = gsi_start_bb(basicBlockForFunctionOne); !gsi_end_p(gsi); gsi_next(&gsi)) {
                                gimpleCountForFunctionOne++;
                            }

                            for (gimple_stmt_iterator gsi = gsi_start_bb(basicBlockForFunctionTwo); !gsi_end_p(gsi); gsi_next(&gsi)) {
                                gimpleCountForFunctionTwo++;
                            }

                            if (gimpleCountForFunctionOne != gimpleCountForFunctionTwo) {
                                identical = false;

                                // Print that the gimple count was not the same
                                fprintf(dump_file,"XXXXX Failed Check #2 (The number of gimple statements differ between two function)\n");
                                break;
                            }

                            // Print that we passed the second check
                            fprintf(dump_file, "Passed Check #2 (The number of GIMPLE counts are the same for each function)\n");

                            // So if it passes the check above we can now compare the statements in parallel
                            gimple_stmt_iterator gsiIteratorForFunctionOne = gsi_start_bb(basicBlockForFunctionOne);
                            gimple_stmt_iterator gsiIteratorForFunctionTwo = gsi_start_bb(basicBlockForFunctionTwo);

                            while(!gsi_end_p(gsiIteratorForFunctionOne) && !gsi_end_p(gsiIteratorForFunctionTwo)) {
                                gimple *statementFromFunctionOne = gsi_stmt(gsiIteratorForFunctionOne);
                                gimple *statementFromFunctionTwo = gsi_stmt(gsiIteratorForFunctionTwo);

                                // Here we can ignore the variable names using gimple code
                                if (gimple_code(statementFromFunctionOne) != gimple_code(statementFromFunctionTwo)){
                                    identical = false;
                                    break;
                                }

                                // Compare operands
                                if (gimple_num_ops(statementFromFunctionOne) != gimple_num_ops(statementFromFunctionTwo)) {
                                    identical =false;
                                    break;
                                }

                                gsi_next(&gsiIteratorForFunctionOne);
                                gsi_next(&gsiIteratorForFunctionTwo);
                            }
                            basicBlockForFunctionOne = basicBlockForFunctionOne->next_bb;
                            basicBlockForFunctionTwo = basicBlockForFunctionTwo->next_bb;
                        }
                    }
                    
                    // Now just print the PRUNE / NOPRUNE MESSAGE
                    if (identical) {
                        fprintf(dump_file, "PRUNE: %s\n", baseName.c_str());
                    }
                    else {
                        fprintf(dump_file, "NOPRUNE: %s\n", baseName.c_str());
                    }

                }
            }
    };
}

// This is used inside the tree-pass.h file
gimple_opt_pass* make_pass_hteli1(gcc::context *ctxt) {
    return new pass_hteli1(ctxt);
}
