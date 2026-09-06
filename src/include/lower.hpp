#ifndef LOWER_HPP_INCLUDED
#define LOWER_HPP_INCLUDED

#include "tac.hpp"

#include <unordered_map>

typedef struct {
        std::unordered_map<std::string, STRING_ID>      string_ids;
        int                                             next_string_id;
        std::vector<TAC_string>                         strings;
} program_state;

typedef struct {
        program_state                           program;
        int                                     next_temp;
        int                                     next_label;
        int                                     current_label;
        std::vector<TAC_instruction *>          current_instrs;
        int                                     current_open;
        std::vector<TAC_block>                  completed_blocks;
        std::unordered_map<int, TAC_operand *>  bindings;
} builder;

#endif // LOWER_HPP_INCLUDED
