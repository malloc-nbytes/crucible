;; INSTALLATION
;;
;; You can either just evaluate this buffer while in Emacs
;; or put it in your ~/.emacs.d/init.el (or a different file and load it).

;; Crucible mode
(defconst crucible-mode-syntax-table
  (with-syntax-table (copy-syntax-table)
    (modify-syntax-entry ?- ". 124b")
    (modify-syntax-entry ?* ". 23")
    (modify-syntax-entry ?\n "> b")
    (modify-syntax-entry ?' "\"")
    (modify-syntax-entry ?' ".")
    (syntax-table))
  "Syntax table for `crucible-mode'.")

;; Function taken from:
;;  https://www.omarpolo.com/post/writing-a-major-mode.html
(defun crucible-indent-line ()
  "Indent current line."
  (let (indent
        boi-p
        move-eol-p
        (point (point)))
    (save-excursion
      (back-to-indentation)
      (setq indent (car (syntax-ppss))
            boi-p (= point (point)))
      (when (and (eq (char-after) ?\n)
                 (not boi-p))
        (setq indent 0))
      (when boi-p
        (setq move-eol-p t))
      (when (or (eq (char-after) ?\))
                (eq (char-after) ?\}))
        (setq indent (1- indent)))
      (delete-region (line-beginning-position)
                     (point))
      (indent-to (* tab-width indent)))
    (when move-eol-p
      (move-end-of-line nil))))

(eval-and-compile
  (defconst crucible-keywords
    '("if" "else" "while" "let"
      "void" "i8" "i16" "i32" "i64" "u8" "u16" "u32" "u64" "bool" "str" "size_t"
      "for" "proc" "return" "mut" "break" "macro" "exit" "extern" "enum"
      "struct" "import" "ref" "end" "export" "embed" "true" "false"
      "def" "in" "null" "type" "module" "where" "continue")))

(defconst crucible-highlights
  `((,(regexp-opt crucible-keywords 'symbols) . font-lock-keyword-face)
    (,(rx (group "--" (zero-or-more (not (any "\n"))))
         (group-n 1 (zero-or-more (any "\n"))))
     (1 font-lock-comment-delimiter-face)
     (2 font-lock-comment-face nil t))))

;;;###autoload
(define-derived-mode crucible-mode prog-mode "crucible"
  "Major Mode for editing Crucible source code."
  :syntax-table crucible-mode-syntax-table
  (setq font-lock-defaults '(crucible-highlights))
  (setq-local comment-start "--")
  (setq-local indent-tabs-mode nil)
  (setq-local tab-width 8)
  (setq-local indent-line-function #'crucible-indent-line)
  (setq-local standard-indent 2))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.cr\\'" . crucible-mode))

(provide 'crucible-mode)
;; End Crucible mode
