" INSTALLATION
"
" 1. Put this file into  the directory: `~/.vim/syntax/`
" 2. In your .vimrc, put the following:
"     autocmd BufRead,BufNewFile *.cr set filetype=crucible
"
" If you are using NeoVim, put this file into the directory: ~/.config/nvim/syntax
" In your init.lua, put the following:
"     vim.cmd [[
"       augroup filetype_crucible
"         autocmd!
"         autocmd BufRead,BufNewFile *.cr set filetype=crucible
"       augroup END
"     ]]

if exists("b:current_syntax")
  finish
endif

set iskeyword=a-z,A-Z,-,*,_,!,@

" Language keywords
syntax keyword CrucibleKeywords if else while let void i8 i16 i32 i64 u8 u16 u32 u64 str for proc return mut break macro exit extern usize struct import ref end export def in null type module where continue embed

" Comments
syntax region CrucibleCommentLine start="--" end="$"

" String literals
syntax region CrucibleString start=/\v"/ skip=/\v\\./ end=/\v"/ contains=CrucibleEscapes

" Char literals
syntax region CrucibleChar start=/\v'/ skip=/\v\\./ end=/\v'/ contains=CrucibleEscapes

" Escape literals \n, \r, ....
syntax match CrucibleEscapes display contained "\\[nr\"']"

highlight default link CrucibleKeywords Keyword
highlight default link CrucibleCommentLine Comment
highlight default link CrucibleString String
highlight default link CrucibleNumber Number
highlight default link CrucibleChar Character
highlight default link CrucibleEscapes SpecialChar

let b:current_syntax = "Crucible"
