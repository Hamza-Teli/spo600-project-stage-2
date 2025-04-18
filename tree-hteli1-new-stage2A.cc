// This pass was created by Hamza Teli with the help of
// Professor Chris Tyler's SPO600 wiki and lecture. 

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

namespace {

    struct FunctionMeta {
        std::string name;
        function *funcPtr;
        size_t gimpleCount;
        size_t basicBlockCount;
    };

    std::map<std::string, FunctionMeta> functionMap;
    std::string base_function;
    static function *base_func_ptr = nullptr;

    const pass_data pass_data_hteli1 = {
        GIMPLE_PASS,
        "hteli1",
        OPTGROUP_NONE,
        TV_NONE,
        PROP_cfg,
        0,
        0,
        0,
        0,
    };

    class pass_hteli1 : public gimple_opt_pass {
    public:
        pass_hteli1(gcc::context *ctxt) : gimple_opt_pass(pass_data_hteli1, ctxt) {}

        bool gate(function *) final override {
            return true;
        }

        unsigned int execute(function *func) override {
            std::string functionName = IDENTIFIER_POINTER(DECL_NAME(func->decl));

            // Detect and store base function name from ".resolver"
            if (base_function.empty() && functionName.find(".resolver") != std::string::npos) {
                base_function = functionName.substr(0, functionName.find(".resolver"));
                if (dump_file) {
                    fprintf(dump_file, "[BASE FOUND] Base function identified: %s\n", base_function.c_str());
                }
            }

            // Skip functions that are not base or clones of base
            if (base_function.empty() || 
                functionName.find(base_function) != 0 || 
                functionName.find(".resolver") != std::string::npos) {
                return 0;
            }

            size_t basicBlockCount = get_number_of_basic_blocks(func);
            size_t gimpleCount = get_number_of_gimple_statements(func);

            functionMap[functionName] = {
                functionName,
                func,
                gimpleCount,
                basicBlockCount
            };

            if (dump_file) {
                fprintf(dump_file, "Function: %s\n", functionName.c_str());
                fprintf(dump_file, "  Basic Blocks: %zu\n", basicBlockCount);
                fprintf(dump_file, "  GIMPLE Stmts: %zu\n", gimpleCount);
                fprintf(dump_file, "---------------------------\n");
            }

            // Record base function pointer
            if (functionName == base_function) {
                base_func_ptr = func;
            }

            // Compare with base function if this is a clone
            if (functionName != base_function && base_func_ptr != nullptr) {
                size_t base_bb = get_number_of_basic_blocks(base_func_ptr);
                size_t base_gimples = get_number_of_gimple_statements(base_func_ptr);

                if (base_bb == basicBlockCount && base_gimples == gimpleCount) {
                    fprintf(dump_file, "[CLONE DETECTED] %s is structurally identical to %s\n", functionName.c_str(), base_function.c_str());
                } else {
                    fprintf(dump_file, "[NO CLONE] %s differs from %s\n", functionName.c_str(), base_function.c_str());
                }
                fprintf(dump_file, "===========================\n");
            }

            return 0;
        }

        size_t get_number_of_basic_blocks(function *func) {
            size_t count = 0;
            basic_block bb;
            FOR_EACH_BB_FN(bb, func) {
                count++;
            }
            return count;
        }

        size_t get_number_of_gimple_statements(function *func) {
            size_t count = 0;
            basic_block bb;
            FOR_EACH_BB_FN(bb, func) {
                for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                    gimple *stmt = gsi_stmt(gsi);
                    if (stmt) count++;
                }
            }

            if (dump_file) {
                fprintf(dump_file, "Number of gimple statements is %zu\n", count);
            }

            return count;
        }
    };
}

gimple_opt_pass* make_pass_hteli1(gcc::context *ctxt) {
    return new pass_hteli1(ctxt);
}
