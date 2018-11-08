# qcir14_to_pcnf
A tool that receives a QBF as input in QCIR-14 format, transforms the formula to PCNF (Prenex Conjunctive Normal Form) and outputs it in the QDIMACS format. Implemented in C.

The program consists of the files data_types.c, data_types.h and main.c.

Usage:
       QCIR14_to_PCNF [-g][-s <prenexing_strategy>] <input_filename>

  -g: Optimal placement of gate variables in prefix.
      By default they are quantified at the innermost block.
  -s: Specify a prenexing strategy:\n"
      '0':∃↑∀↑ (default), '1':∃↑∀↓, '2':∃↓∀↑, '3':∃↓∀↓

This is a Bachelor Thesis work, assigned by
Associate Prof. Dipl.-Math. Dr.techn. Florian Zuleger and
Ao.Univ.Prof. Dipl.-Ing. Dr.rer.nat. Uwe Egly of TU Wien.
Written by Georgi Marinov(gogmarinov@gmail.com)
with the wonderful help and provision of God.
