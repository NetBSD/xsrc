;;
;; Copyright (c) 2001 by The XFree86 Project, Inc.
;;
;; Permission is hereby granted, free of charge, to any person obtaining a
;; copy of this software and associated documentation files (the "Software"),
;; to deal in the Software without restriction, including without limitation
;; the rights to use, copy, modify, merge, publish, distribute, sublicense,
;; and/or sell copies of the Software, and to permit persons to whom the
;; Software is furnished to do so, subject to the following conditions:
;;
;; The above copyright notice and this permission notice shall be included in
;; all copies or substantial portions of the Software.
;;  
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
;; THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
;; OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.
;;
;; Except as contained in this notice, the name of the XFree86 Project shall
;; not be used in advertising or otherwise to promote the sale, use or other
;; dealings in this Software without prior written authorization from the
;; XFree86 Project.
;;
;; Author: Paulo César Pereira de Andrade
;;
;;
;; $XFree86: xc/programs/xedit/lisp/modules/fun.lsp,v 1.5 2001/10/20 00:19:36 paulo Exp $
;;
(provide "fun")

(defun caar (a)		(car (car a)))
(defun cadr (a)		(nth 1 a))
(defun cdar (a)		(cdr (car a)))
(defun cddr (a)		(nthcdr 2 a))
(defun caaar (a)	(car (car (car a))))
(defun caadr (a)	(car (car (cdr a))))
(defun cadar (a)	(car (cdr (car a))))
(defun caddr (a)	(nth 2 a))
(defun cdaar (a)	(cdr (car (car a))))
(defun cdadr (a)	(cdr (car (cdr a))))
(defun cddar (a)	(cdr (cdr (car a))))
(defun cdddr (a)	(nthcdr 3 a))
(defun caaaar (a)	(car (car (car (car a)))))
(defun caaadr (a)	(car (car (car (cdr a)))))
(defun caadar (a)	(car (car (cdr (car a)))))
(defun caaddr (a)	(car (car (cdr (cdr a)))))
(defun cadaar (a)	(car (cdr (car (car a)))))
(defun cadadr (a)	(car (cdr (car (cdr a)))))
(defun caddar (a)	(car (cdr (cdr (car a)))))
(defun cadddr (a)	(nth 3 a))
(defun cdaaar (a)	(cdr (car (car (car a)))))
(defun cdaadr (a)	(cdr (car (car (cdr a)))))
(defun cdadar (a)	(cdr (car (cdr (car a)))))
(defun cdaddr (a)	(cdr (car (cdr (cdr a)))))
(defun cddaar (a)	(cdr (cdr (car (car a)))))
(defun cddadr (a)	(cdr (cdr (car (cdr a)))))
(defun cdddar (a)	(cdr (cdr (cdr (car a)))))
(defun cddddr (a)	(nthcdr 4 a))

(defun second (a)	(nth 1 a))
(defun third (a)	(nth 2 a))
(defun fourth (a)	(nth 3 a))
(defun fifth (a)	(nth 4 a))
(defun sixth (a)	(nth 5 a))
(defun seventh (a)	(nth 6 a))
(defun eighth (a)	(nth 7 a))
(defun ninth (a)	(nth 8 a))
(defun tenth (a)	(nth 9 a))

(defun copy-seq (sequence)	(subseq sequence 0))

(defmacro push (object place)
    (list 'setf place (list 'cons object place)))

(defmacro pop (place)
    (list 'prog1 (list 'car place) (list 'setf place (list 'cdr place))))

(defmacro prog (init &rest body)
    `(block nil (let ,init (tagbody ,@body))))

(defmacro prog* (init &rest body)
    `(block nil (let* ,init (tagbody ,@body))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; setf
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defsetf car (list) (value)	`(progn (rplaca ,list ,value) ,value))
(defsetf cdr (list) (value)	`(progn (rplacd ,list ,value) ,value))

(defsetf caar (list) (value)	`(progn (rplaca (car ,list) ,value) ,value))
(defsetf cadr (list) (value)	`(progn (rplaca (cdr ,list) ,value) ,value))
(defsetf cdar (list) (value)	`(progn (rplacd (car ,list) ,value) ,value))
(defsetf cddr (list) (value)	`(progn (rplacd (cdr ,list) ,value) ,value))
(defsetf caaar (list) (value)	`(progn (rplaca (caar ,list) ,value) ,value))
(defsetf caadr (list) (value)	`(progn (rplaca (cadr ,list) ,value) ,value))
(defsetf cadar (list) (value)	`(progn (rplaca (cdar ,list) ,value) ,value))
(defsetf caddr (list) (value)	`(progn (rplaca (cddr ,list) ,value) ,value))
(defsetf cdaar (list) (value)	`(progn (rplacd (caar ,list) ,value) ,value))
(defsetf cdadr (list) (value)	`(progn (rplacd (cadr ,list) ,value) ,value))
(defsetf cddar (list) (value)	`(progn (rplacd (cdar ,list) ,value) ,value))
(defsetf cdddr (list) (value)	`(progn (rplacd (cddr ,list) ,value) ,value))
(defsetf caaaar (list) (value)	`(progn (rplaca (caaar ,list) ,value) ,value))
(defsetf caaadr (list) (value)	`(progn (rplaca (caadr ,list) ,value) ,value))
(defsetf caadar (list) (value)	`(progn (rplaca (cadar ,list) ,value) ,value))
(defsetf caaddr (list) (value)	`(progn (rplaca (caddr ,list) ,value) ,value))
(defsetf cadaar (list) (value)	`(progn (rplaca (cdaar ,list) ,value) ,value))
(defsetf cadadr (list) (value)	`(progn (rplaca (cdadr ,list) ,value) ,value))
(defsetf caddar (list) (value)	`(progn (rplaca (cddar ,list) ,value) ,value))
(defsetf cadddr (list) (value)	`(progn (rplaca (cdddr ,list) ,value) ,value))
(defsetf cdaaar (list) (value)	`(progn (rplacd (caaar ,list) ,value) ,value))
(defsetf cdaadr (list) (value)	`(progn (rplacd (caadr ,list) ,value) ,value))
(defsetf cdadar (list) (value)	`(progn (rplacd (cadar ,list) ,value) ,value))
(defsetf cdaddr (list) (value)	`(progn (rplacd (caddr ,list) ,value) ,value))
(defsetf cddaar (list) (value)	`(progn (rplacd (cdaar ,list) ,value) ,value))
(defsetf cddadr (list) (value)	`(progn (rplacd (cdadr ,list) ,value) ,value))
(defsetf cdddar (list) (value)	`(progn (rplacd (cddar ,list) ,value) ,value))
(defsetf cddddr (list) (value)	`(progn (rplacd (cdddr ,list) ,value) ,value))

(defsetf first (list) (value)	`(progn (rplaca ,list ,value) ,value))
(defsetf second (list) (value)	`(progn (rplaca (nthcdr 1 ,list) ,value) ,value))
(defsetf third (list) (value)	`(progn (rplaca (nthcdr 2 ,list) ,value) ,value))
(defsetf fourth (list) (value)	`(progn (rplaca (nthcdr 3 ,list) ,value) ,value))
(defsetf fifth (list) (value)	`(progn (rplaca (nthcdr 4 ,list) ,value) ,value))
(defsetf sixth (list) (value)	`(progn (rplaca (nthcdr 5 ,list) ,value) ,value))
(defsetf seventh (list) (value)	`(progn (rplaca (nthcdr 6 ,list) ,value) ,value))
(defsetf eighth (list) (value)	`(progn (rplaca (nthcdr 7 ,list) ,value) ,value))
(defsetf ninth (list) (value)	`(progn (rplaca (nthcdr 8 ,list) ,value) ,value))
(defsetf tenth (list) (value)	`(progn (rplaca (nthcdr 9 ,list) ,value) ,value))

(defsetf rest (list) (value)	`(progn (rplacd ,list ,value) ,value))

(defun xedit::nth-store (index list value)
    (rplaca (nthcdr index list) value) value)
(defsetf nth xedit::nth-store)

(defsetf aref (array &rest indices) (value)
    `(xedit::vector-store ,array ,@indices ,value))

(defsetf get (symbol key &optional default) (value)
    `(xedit::put ,symbol ,key ,value))

(defsetf char xedit::char-store)
(defsetf schar xedit::char-store)
(defsetf elt xedit::elt-store)

(defsetf subseq (sequence start &optional end) (value)
    `(progn (replace ,sequence ,value :start1 ,start :end1 ,end) ,value))
