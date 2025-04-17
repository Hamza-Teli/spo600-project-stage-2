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
                        const char *name_of_function = IDENTIFIER_POINTER(DECL_NAME(current_function_pointer->decl));
                        std::string functionName(name_of_function);
                        
                        // Split the last dot
                        size_t pos = functionName.rfind('.');
                        if (pos == std::string::npos) {
                            continue;
                        }
                        std::string baseName = functionName.substr(0,pos);
                        std::string suffix = functionName.substr(pos+1);
                        
                        // Now we check that if the function has a resolver suffix, we simply store its base name
                        if (suffix == "resolver") {
                          //std::string baseName = functionName.substr(0, dot);
                          resolverMap[baseName] = functionName;

                          if (dump_file) {
                                // Show an output
                          fprintf(dump_file, "--------------------------------------------------------------------------------\n");
                          fprintf(dump_file, "**** ---> Resolver was found for base function: %s\n", baseName.c_str());
                          }
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
                        const char *name_of_function = IDENTIFIER_POINTER(DECL_NAME(current_function_pointer->decl));
                        std::string functionName(name_of_function);

                        // split last dot again
                        size_t pos = functionName.rfind('.');
                        if (pos == std::string::npos) {
                            continue;
                        }
                        std::string baseName = functionName.substr(0,pos);
                        std::string suffix = functionName.substr(pos+1);
                        
                        // Now we check that if the function has a resolver suffix, if so just continue
                        if (suffix == "resolver") {  
                            continue;
                        }
                
                        // Now we check if base has a resolver
                        if (resolverMap.count(baseName)) {
                            variantMap[baseName].push_back(functionName);
                            if (dump_file) {
                                fprintf(dump_file, "**** ---> Clone variant successfully found: %s (base function: %s) with resolver: %s\n", functionName.c_str(), baseName.c_str(), resolverMap[baseName].c_str());
                            fprintf(dump_file, "--------------------------------------------------------------------------------\n");
                            }
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
                        const char *function_name_n = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
                        std::string functionName(function_name_n);
                        
                        // 
                        if (functionName == variantValues[0]) {
                            functionOne = fun;
                        }
                        else if (functionName == variantValues[1]) {
                            functionTwo = fun;
                        }
                    }

                    // Validate if both are valid
                    if (!functionOne || !functionTwo) {
                        fprintf(dump_file, "ERROR! Function 1 and function 2 could not be retrieved. Check the logic!\n");
                        continue;
                    }
                    else 
                    {
                        fprintf(dump_file, "Successfully retrieved both function 1 and two, the addresses are as follows: %p,  %p \n", (void*)functionOne, (void*)functionTwo);
                    }

                    // Function names
                    std::string functionOneName = function_name(functionOne);
                    std::string functionTwoName = function_name(functionTwo);

                    // Basic Block Section -----------------------------------------------------------------------------------------------
                    // Get number of basic blocks
                    size_t basicBlockCountForFunctionOne = get_number_of_basic_blocks(functionOne);
                    size_t basicBlockCountForFunctionTwo = get_number_of_basic_blocks(functionTwo);

                    // Lets print a basic block count for each function section here
                    fprintf(dump_file, "\n---------------------------------------------------------\n");
                    fprintf(dump_file, "----------------- Basic Block Counts ---------------------\n");
                    fprintf(dump_file, "Function 1: (%s) Block Count %zu\n", functionOneName.c_str(), basicBlockCountForFunctionOne);
                    fprintf(dump_file, "Function 2: (%s) Block Count %zu\n", functionTwoName.c_str(), basicBlockCountForFunctionTwo);
                    fprintf(dump_file, "---------------------------------------------------------\n");

                    // Number of gimple statement Section -----------------------------------------------------------------------------------------------
                    // Use the helper function I created
                    size_t gimpleCountForFunctionOne = get_number_of_gimple_statements(functionOne);
                    size_t gimpleCountForFunctionTwo = get_number_of_gimple_statements(functionTwo);

                    // Lets print a basic block count for each function section here
                    fprintf(dump_file, "\n---------------------------------------------------------\n");
                    fprintf(dump_file, "----------------- GIMPLE Counts ---------------------\n");
                    fprintf(dump_file, "Function 1: (%s) Gimple Count %zu\n", functionOneName.c_str(), gimpleCountForFunctionOne);
                    fprintf(dump_file, "Function 2: (%s) Gimple Count %zu\n", functionTwoName.c_str(), gimpleCountForFunctionTwo);
                    fprintf(dump_file, "---------------------------------------------------------\n");


                    // GIMPLE check -----------------------------------------------------------------------------------------------
                    bool gimpleCodeAndOpsCheck =  are_functions_identical_by_gimple_code_and_ops(functionOne, functionTwo);
                    fprintf(dump_file, "%d for gimpleCodeAndOpsCheck. (1 means pass, 0 means fail)\n", gimpleCodeAndOpsCheck);

                    // Set a flag
                    bool identical = true;

                    // Final flag set
                    if (basicBlockCountForFunctionTwo != basicBlockCountForFunctionOne || gimpleCountForFunctionTwo != gimpleCountForFunctionOne || !gimpleCodeAndOpsCheck) {
                        identical = false;
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

            // Get the number of basic blocks
            size_t get_number_of_basic_blocks(function *func) {
                // Instantiate counter
                size_t count = 0;

                basic_block bb;
                FOR_EACH_BB_FN(bb, func){
                    count++;
                }

                return count;
            }

            // Get the number of statements inside the function
            size_t get_number_of_gimple_statements(function *func) {
                // Initialize basic block
                basic_block bb;

                // Hold number of GIMPLE values
                size_t count = 0;

                FOR_EACH_BB_FN(bb, func) {
                    for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                        count++;
                    }
                }
                return count;
            }

            // This function performs the gimple code check by checking the operands and number of statements
            bool are_functions_identical_by_gimple_code_and_ops(function *f1, function *f2){
                basic_block bb;

                // Create vector of gimple to store statements
                std::vector<gimple *> stmts1, stmts2;

                // Collect all statements from function 1
                FOR_EACH_BB_FN(bb, f1)
                {
                    for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
                    {
                        stmts1.push_back(gsi_stmt(gsi));
                    }
                }

                // Collect all statements from function 2
                FOR_EACH_BB_FN(bb, f2)
                {
                    for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
                    {
                        stmts2.push_back(gsi_stmt(gsi));
                    }
                }

                // Compare rhe statement counts
                if (stmts1.size() != stmts2.size())
                {
                    fprintf(dump_file, "Statement count mismatch: %zu vs %zu\n", stmts1.size(), stmts2.size());
                    return false;
                }

                // Compare GIMPLE codes and operand counts
                for (size_t i = 0; i < stmts1.size(); ++i)
                {
                    gimple *s1 = stmts1[i];
                    gimple *s2 = stmts2[i];

                    if (gimple_code(s1) != gimple_code(s2)) {
                        return false;
                    }

                    unsigned ops = gimple_num_ops(s1);
                    if (ops != gimple_num_ops(s2)) {
                        return false;

                    }

                    for (unsigned j = 0; j < ops; ++j) {
                        if (TREE_TYPE(gimple_op(s1, j)) != TREE_TYPE(gimple_op(s2, j)))
                            return false;
                    }
                }
                return true;
            }

    };
}

// This is used inside the tree-pass.h file
gimple_opt_pass* make_pass_hteli1(gcc::context *ctxt) {
    return new pass_hteli1(ctxt);
}
